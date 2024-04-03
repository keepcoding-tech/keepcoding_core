// This file is part of keepcoding_core
// ==================================
//
// socket.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * a network struct
 */

#ifndef KC_SOCKET_T_H
#define KC_SOCKET_T_H

#include "../system/logger.h"
#include "server.h"

#include <stdio.h>
#include <stdbool.h>

//---------------------------------------------------------------------------//

struct kc_socket_t
{
  int fd;                      // the file descriptor
  struct sockaddr_in* addr;    // socket address

  int error;     // the error code returned by "accept"
  bool success;  // state of the success
};

struct kc_socket_t* new_socket      (void);
void                destroy_socket  (struct kc_socket_t* socket);

//---------------------------------------------------------------------------//

#endif /* KC_SOCKET_T_H */
