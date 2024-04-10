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

struct kc_server_t* new_server_IPv4  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server_IPv6  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server       (const int AF, const char* IP, const unsigned int PORT);
int                 start_server     (struct kc_server_t* self);

void* dispatch_server  (void* socket_fd);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static int _accept_connection  (int server_fd, struct kc_socket_t* socket);

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
  // create a server instance to be returned
  struct kc_server_t* new_server = malloc(sizeof(struct kc_server_t));
  if (new_server == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // create a new socket
  new_server->socket = new_socket(AF, IP, PORT);
  if (new_server->socket == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the server first
    free(new_server);

    return NULL;
  }

  // allocate memory for the logger
  logger = new_logger(KC_SERVER_LOG_PATH);
  if (logger == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the server and socket
    destroy_socket(new_server->socket);
    free(new_server);

    return NULL;
  }

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

  destroy_logger(logger);
  destroy_socket(server->socket);
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
  int ret = bind(self->socket->fd, (struct sockaddr*)self->socket->addr, sizeof(*self->socket->addr));
  if (ret != KC_SUCCESS)
  {
    logger->log(logger, KC_FATAL_LOG,
      KC_NETWORK_ERROR, __FILE__, __LINE__, __func__);
    return KC_NETWORK_ERROR;
  }

  // start listening for connections
  ret = listen(self->socket->fd, KC_SERVER_MAX_CONNECTIONS);
  if (ret != KC_SUCCESS)
  {
    logger->log(logger, KC_FATAL_LOG,
      KC_LOST_CONNECTION, __FILE__, __LINE__, __func__);
    return KC_LOST_CONNECTION;
  }

  printf("\nApplication listening on %s:%d ... \n\n", self->socket->ip, self->socket->port);

  // separate the connections on different threads
  while (1)
  {
    // create a new socket for new connections
    struct kc_socket_t socket;

    // accept the connection for the new socket
    int ret = _accept_connection(self->socket->fd, &socket);
    if (ret != KC_SUCCESS)
    {
      logger->log(logger, KC_FATAL_LOG,
        ret, __FILE__, __LINE__, __func__);
      return ret;
    }

    // add the socket to the client list
    connections[conn_count++] = &socket;

    void* fd = &socket.fd;
    pthread_t id;

    // create a new thread to process the new connection
    pthread_create(&id, NULL, &dispatch_server, fd);
  }

  // first shutdown the connections
  // then return the error, if any
  shutdown(self->socket->fd, SHUT_RDWR);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

void* dispatch_server(void* socket_fd)
{
  char recv_buffer[1024];

  // receive the message from the connection
  ssize_t recv_ret = recv(*(int*)socket_fd, recv_buffer, 1024, 0);
  if (recv_ret <= KC_SUCCESS)
  {
    return (void*)KC_INVALID;
  }

  // print the message
  recv_buffer[recv_ret] = '\0';
  printf("%s \n", recv_buffer);

  char* response =
    "HTTP/1.1 200 OK\n"
    "Content-Type: text/plain\n"
    "\n"
    "Hello, world!\n";

  // send a HTTP response
  send(*(int*)socket_fd, response, strlen(response), 0);

  // close the socket
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

  // make sure the connection was made succesfully
  if (client_fd <= 0)
  {
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
