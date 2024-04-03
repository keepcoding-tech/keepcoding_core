// This file is part of keepcoding_core
// ==================================
//
// server.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/network/server.h"
#include "../../hdrs/common.h"

#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int start_server   (struct kc_server_t* self);
int stop_server    (struct kc_server_t* self);
int listen_server  (int server_fd);

void* dispatch_server  (void* socket_fd);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static int _accept_connection  (int server_fd, struct kc_socket_t* socket);

static void _send_message  (char* buffer, int socket_fd);
struct kc_socket_t socket_list[10];
int socket_list_count = 0;

//---------------------------------------------------------------------------//

struct kc_server_t* new_server(const char* IP, const int PORT)
{
  // create a server instance to be returned
  struct kc_server_t* new_server = malloc(sizeof(struct kc_server_t));

  // confirm that there is memory to allocate
  if (new_server == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return NULL;
  }

  // create socket
  new_server->fd = socket(AF_INET, SOCK_STREAM, 0);

  // bind to open port
  new_server->addr = malloc(sizeof(struct sockaddr_in));
  if (new_server->addr == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return NULL;
  }

  new_server->addr->sin_family = AF_INET;
  new_server->addr->sin_port = htons(PORT);

  // asign a default IP address
  if (strlen(IP) <= 0 || IP == NULL)
  {
    new_server->addr->sin_addr.s_addr = INADDR_ANY;
  }
  else
  {
    inet_pton(AF_INET, IP, &new_server->addr->sin_addr.s_addr);
  }

  new_server->ip = inet_ntoa(new_server->addr->sin_addr);

  new_server->start    = start_server;
  new_server->stop     = stop_server;
  new_server->dispatch = dispatch_server;
  new_server->listen   = listen_server;

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

  for (int i = 0; i < socket_list_count; ++i)
  {
    destroy_socket(&socket_list[i]);
  }

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
  int ret = bind(self->fd, (struct sockaddr*)self->addr, sizeof(*self->addr));
  if (ret != KC_SUCCESS)
  {
    printf("\n%s\n", strerror(errno));
    return KC_LOST_CONNECTION;
  }

  // start listening for connection
  ret = listen(self->fd, 10);
  if (ret != KC_SUCCESS)
  {
    return KC_LOST_CONNECTION;
  }

  printf("Application listening on %s ... \n", self->ip);

  // separate the connections on different threads
  ret = self->listen(self->fd);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  // first shutdown the connections
  // then return the error, if any
  shutdown(self->fd, SHUT_RDWR);

  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int stop_server(struct kc_server_t* self)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int listen_server(int server_fd)
{
  while (1)
  {
    // create a new socket for new connections
    struct kc_socket_t* socket = new_socket();

    // accept the connection for the new socket
    int ret = _accept_connection(server_fd, socket);
    if (ret != KC_SUCCESS)
    {
      break;
    }

    // add the socket to the client list
    socket_list[socket_list_count++] = *socket;

    void* fd = &socket->fd;
    pthread_t id;

    // create a new thread to process the new connection
    pthread_create(&id, NULL, &dispatch_server, fd);
  }

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
      break;
    }

    // print the message
    recv_buffer[recv_ret] = '\0';
    printf("received: %s", recv_buffer);

    // send message to the other clients
    _send_message(recv_buffer, *(int*)socket_fd);
  }

  close(*(int*)socket_fd);

  return (void*)KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _accept_connection(int server_fd, struct kc_socket_t* socket)
{
  // accept the connection
  struct sockaddr_in client_addr;
  int client_addr_size = sizeof(struct sockaddr_in);
  int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t * restrict)&client_addr_size);

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

void _send_message(char* buffer, int socket_fd)
{
  for (int i = 0; i < socket_list_count; ++i)
  {
    if (socket_list[i].fd != socket_fd)
    {
      send(socket_list[i].fd, buffer, strlen(buffer), 0);
    }
  }
}

//---------------------------------------------------------------------------//
