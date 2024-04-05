// This file is part of keepcoding_core
// ==================================
//
// server.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/network/server.h"
#include "../../hdrs/common.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

struct kc_server_t* new_server  (const int AF, const char* IP, const unsigned int PORT);

int start_server   (struct kc_server_t* self);

void* dispatch_server  (void* socket_fd);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static int _accept_connection  (int server_fd, struct kc_socket_t* socket);
static int _validate_port      (const unsigned int port);
static int _validate_ip        (const int AF, const char* ip);

//--- MARK: PRIVATE MEMBERS -------------------------------------------------//

// private member for logging
static struct kc_logger_t* logger;

// keeps track of all the connections
static struct kc_socket_t* connections[KC_SERVER_MAX_CONNECTIONS];
static int conn_count;

//---------------------------------------------------------------------------//

struct kc_server_t* new_server_IPv4(const char* IP, const unsigned int PORT)
{
  // initialize the server for IPv4
  return new_server(AF_INET, IP, PORT);
}

//---------------------------------------------------------------------------//

struct kc_server_t* new_server_IPv6(const char* IP, const unsigned int PORT)
{
  // initialize the server for IPv6
  return new_server(AF_INET6, IP, PORT);
}

//---------------------------------------------------------------------------//

struct kc_server_t* new_server(const int AF, const char* IP, const unsigned int PORT)
{
  // before anything, validate the IP and PORT
  if (_validate_port(PORT) != KC_SUCCESS || _validate_ip(AF, IP) != KC_SUCCESS)
  {
    return NULL;
  }

  // create a server instance to be returned
  struct kc_server_t* new_server = malloc(sizeof(struct kc_server_t));
  if (new_server == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // allocate memory for the socket
  new_server->addr = malloc(sizeof(struct sockaddr_in));
  if (new_server->addr == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the server
    free(new_server);

    return NULL;
  }

  // allocate memory for the logger
  logger = new_logger(KC_SERVER_LOG_PATH);
  if (logger == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the server and socket
    free(new_server->addr);
    free(new_server);

    return NULL;
  }

  // create socket
  new_server->file_descriptor = socket(AF, SOCK_STREAM, 0);

  // assign IP and PORT
  inet_pton(AF, IP, &new_server->addr->sin_addr.s_addr);
  new_server->addr->sin_family = AF;
  new_server->addr->sin_port = htons(PORT);

  // save the ip and port for easy access
  new_server->ip = inet_ntoa(new_server->addr->sin_addr);
  new_server->port = PORT;

  // asign public member functions
  new_server->start = start_server;

  return new_server;
}

//---------------------------------------------------------------------------//

void destroy_server(struct kc_server_t* server)
{
  if (server == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  for (int i = 0; i < conn_count; ++i)
  {
    destroy_socket(connections[i]);
  }

  free(server->addr);
  free(server);
}

//---------------------------------------------------------------------------//

int start_server(struct kc_server_t* self)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // bind the server socket to the IP address
  int ret = bind(self->file_descriptor, (struct sockaddr*)self->addr, sizeof(*self->addr));
  if (ret != KC_SUCCESS)
  {
    logger->log(logger, KC_FATAL_LOG,
      KC_NETWORK_ERROR, __FILE__, __LINE__, __func__);
    return KC_NETWORK_ERROR;
  }

  // start listening for connections
  ret = listen(self->file_descriptor, KC_SERVER_MAX_CONNECTIONS);
  if (ret != KC_SUCCESS)
  {
    logger->log(logger, KC_FATAL_LOG,
      KC_LOST_CONNECTION, __FILE__, __LINE__, __func__);
    return KC_LOST_CONNECTION;
  }

  printf("\nApplication listening on http://%s:%d ... \n", self->ip, self->port);

  // separate the connections on different threads
  while (1)
  {
    // create a new socket for new connections
    struct kc_socket_t* socket = new_socket();

    // accept the connection for the new socket
    int ret = _accept_connection(self->file_descriptor, socket);
    if (ret != KC_SUCCESS)
    {
      logger->log(logger, KC_FATAL_LOG,
        ret, __FILE__, __LINE__, __func__);
      return ret;
    }

    // add the socket to the client list
    connections[conn_count++] = socket;

    void* fd = &socket->fd;
    pthread_t id;

    // create a new thread to process the new connection
    pthread_create(&id, NULL, &dispatch_server, fd);
  }

  // first shutdown the connections
  // then return the error, if any
  shutdown(self->file_descriptor, SHUT_RDWR);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

void* dispatch_server(void* socket_fd)
{
  char recv_buffer[1024];

  while (1)
  {

    // receive the message from the connection
    ssize_t recv_ret = recv(*(int*)socket_fd, recv_buffer, 1024, 0);
    if (recv_ret <= KC_SUCCESS)
    {
      // TODO: delete the connection from the list
      break;
    }

    // print the message
    recv_buffer[recv_ret] = '\0';
    printf("received: %s", recv_buffer);

    // send message to the other clients
    for (int i = 0; i < conn_count; ++i)
    {
      if (connections[i]->fd != *(int*)socket_fd)
      {
        send(connections[i]->fd, recv_buffer, strlen(recv_buffer), 0);
      }
    }
  }

  close(*(int*)socket_fd);

  return (void*)KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _accept_connection(int server_fd, struct kc_socket_t* socket)
{
  // create a new socket for the client
  struct sockaddr_in client_addr;
  int client_addr_size = sizeof(struct sockaddr_in);

  // accept the connection
  int client_fd = accept(server_fd, (struct sockaddr*)&client_addr,
    (socklen_t * restrict)&client_addr_size);

  // serialize the data
  socket->addr = &client_addr;
  socket->fd = client_fd;
  socket->success = (bool)(client_fd > 0);
  socket->error = KC_SUCCESS;

  // make sure the connection was made succesfully
  if (socket->success == false)
  {
    socket->error = client_fd;
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

static int _validate_port(const unsigned int port)
{
  // if below 0, is invalid
  if (port < 0)
  {
    log_error("Port number cannot be negative.");
    return KC_INVALID;
  }

  // if the specified port is below the
  // well known service rage, return warning
  if (port < PORT_WELL_KNOWN_SERVICE)
  {
    log_error("Port number below 1024: privileged range.");
    return KC_INVALID;
  }

  // if the specified port between the available
  // and dynamic or private rage, return warning
  if (PORT_AVAILABLE_FOR_USER < port && port < PORT_DYNAMIC_OR_PRIVATE)
  {
    log_error("Port number in dynamic/private range (49152-65535).");
    return KC_INVALID;
  }

  // if above 65535, is invalid
  if (port > PORT_DYNAMIC_OR_PRIVATE)
  {
    log_error("Port number exceeds maximum allowed (65535).");
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

static int _validate_ip(int AF, const char* ip)
{
  unsigned char ip_address[sizeof(struct in6_addr)];
  int ret = inet_pton(AF, ip, ip_address);

  // if the specified ip has an invalid format, is invalid
  if (ret == IP_INVALID_NETWORK_ADDRESS)
  {
    log_error("Invalid network IP address format.");
    return KC_INVALID;
  }

  // if the specified ip is not IPv4 or IPv6, is invalid
  if (ret == IP_INVALID_FAMILY_ADDRESS)
  {
    log_error("Invalid family address: must be IPv4 or IPv6.");
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
