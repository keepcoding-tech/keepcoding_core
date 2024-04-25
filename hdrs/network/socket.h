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

#include "server.h"

#include <stdio.h>
#include <stdbool.h>

//---------------------------------------------------------------------------//

#define PORT_WELL_KNOWN_SERVICE                                            1024
#define PORT_AVAILABLE_FOR_USER                                           49151
#define PORT_DYNAMIC_OR_PRIVATE                                           65535

//---------------------------------------------------------------------------//

struct kc_socket_t
{
  int fd;                      // the file descriptor
  struct sockaddr_in* addr;    // socket address

  char* ip;                    // server IP address
  unsigned int port;           // server PORT
};

struct kc_socket_t* new_socket      (const int AF, const char* ip, const unsigned int port);
void                destroy_socket  (struct kc_socket_t* socket);

//---------------------------------------------------------------------------//

#endif /* KC_SOCKET_T_H */
