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

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int start_client  (struct kc_client_t* self);
int stop_client   (struct kc_client_t* self);

void* listen_client  (void* fd);

//---------------------------------------------------------------------------//

struct kc_client_t* new_client(const char* IP, const int PORT)
{
  // create a client instance to be returned
  struct kc_client_t* new_client = malloc(sizeof(struct kc_client_t));

  // confirm that there is memory to allocate
  if (new_client == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return NULL;
  }

  // create socket
  new_client->fd = socket(AF_INET, SOCK_STREAM, 0);

  // bind to open port
  new_client->addr = malloc(sizeof(struct sockaddr_in));
  if (new_client->addr == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return NULL;
  }

  new_client->addr->sin_family = AF_INET;
  new_client->addr->sin_port = htons(PORT);
  inet_pton(AF_INET, IP, &new_client->addr->sin_addr.s_addr);

  new_client->start = start_client;
  new_client->stop  = stop_client;

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

  free(client->addr);
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

  int ret = connect(self->fd, (struct sockaddr*)self->addr, sizeof(*self->addr));
  if (ret != KC_SUCCESS)
  {
    return KC_LOST_CONNECTION;
  }

  printf("connected\n\n");

  // create a new thread to listen for incoming messages
  pthread_t id;
  void* fd = &self->fd;
  pthread_create(&id, NULL, &listen_client, fd);

  char* send_buffer = NULL;
  size_t buff_len = 0;

  while (1)
  {
    ssize_t len = getline(&send_buffer, &buff_len, stdin);
    ssize_t send_ret = send(self->fd, send_buffer, len, 0);
    if (send_ret <= KC_INVALID)
    {
      return (int)send_ret;
    }

    if (strcmp(send_buffer, "exit\n") == 0)
    {
      break;
    }
  }

  close(self->fd);

  // char* send_buffer = "GET \\ HTTP/1.1\r\nHost:google.com\r\n\r\n";
  // send(self->fd, send_buffer, strlen(send_buffer), 0);

  // char recv_buffer[1024];
  // recv(self->fd, recv_buffer, 1024, 0);

  // printf("received: %s \n", recv_buffer);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int stop_client(struct kc_client_t* self)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

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
