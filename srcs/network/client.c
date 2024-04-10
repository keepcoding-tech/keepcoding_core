// This file is part of keepcoding_core
// ==================================
//
// client.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/network/client.h"
#include "../../hdrs/common.h"

#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

struct kc_client_t* new_client_IPv4  (const char* IP, const int PORT);
struct kc_client_t* new_client_IPv6  (const char* IP, const int PORT);
struct kc_client_t* new_client       (const int AF, const char* IP, const int PORT);
int                 start_client     (struct kc_client_t* self);

void* listen_client  (void* fd);

//---------------------------------------------------------------------------//

struct kc_client_t* new_client_IPv4(const char* IP, const int PORT)
{
  // initialize the client for IPv4
  return new_client(AF_INET, IP, PORT);
}

//---------------------------------------------------------------------------//

struct kc_client_t* new_client_IPv6(const char* IP, const int PORT)
{
  // initialize the client for IPv6
  return new_client(AF_INET6, IP, PORT);
}

//---------------------------------------------------------------------------//

struct kc_client_t* new_client(const int AF, const char* IP, const int PORT)
{
  // create a client instance to be returned
  struct kc_client_t* new_client = malloc(sizeof(struct kc_client_t));

  // confirm that there is memory to allocate
  if (new_client == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // bind to open port
  new_client->socket = new_socket(AF, IP, PORT);
  if (new_client->socket == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the instance first
    free(new_client);

    return NULL;
  }

  // asign public member functions
  new_client->start = start_client;

  return new_client;
}

//---------------------------------------------------------------------------//

void destroy_client(struct kc_client_t* client)
{
  if (client == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  destroy_socket(client->socket);
  free(client);
}

//---------------------------------------------------------------------------//

int start_client(struct kc_client_t* self)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  int ret = connect(self->socket->fd, (struct sockaddr*)self->socket->addr, sizeof(*self->socket->addr));
  if (ret != KC_SUCCESS)
  {
    return KC_LOST_CONNECTION;
  }

  printf("connected\n\n");

  // create a new thread to listen for incoming messages
  pthread_t id;
  void* fd = &self->socket->fd;
  pthread_create(&id, NULL, &listen_client, fd);

  char* send_buffer = NULL;
  size_t buff_len = 0;

  while (1)
  {
    ssize_t len = getline(&send_buffer, &buff_len, stdin);
    ssize_t send_ret = send(self->socket->fd, send_buffer, len, 0);
    if (send_ret <= KC_INVALID)
    {
      return (int)send_ret;
    }

    if (strcmp(send_buffer, "exit\n") == 0)
    {
      break;
    }
  }

  close(self->socket->fd);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

void* listen_client(void* fd)
{
  char recv_buffer[1024];

  while (1)
  {
    // receive the message from the connection
    ssize_t recv_ret = recv(*(int*)fd, recv_buffer, 1024, 0);
    if (recv_ret <= KC_SUCCESS)
    {
      break;
    }

    // print the message
    recv_buffer[recv_ret] = '\0';
    printf("received: %s", recv_buffer);
  }

  return (void*)KC_SUCCESS;
}

//---------------------------------------------------------------------------//
