// This file is part of keepcoding_core
// ==================================
//
// socket.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/network/socket.h"
#include "../../hdrs/common.h"

#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

//---------------------------------------------------------------------------//

struct kc_socket_t* new_socket(void)
{
  // create a new instance to be returned
  struct kc_socket_t* new_socket = malloc(sizeof(struct kc_socket_t));

  // make sure there is momory to alocate
  if (new_socket == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // initialize with default values
  new_socket->fd = 0;
  new_socket->error = KC_SUCCESS;
  new_socket->success = true;

  return new_socket;
}

//---------------------------------------------------------------------------//

void destroy_socket  (struct kc_socket_t* socket)
{
  if (socket == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(socket);
}

//---------------------------------------------------------------------------//
