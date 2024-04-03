// This file is part of keepcoding_core
// ==================================
//
// server.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * a network struct
 */

#ifndef KC_SERVER_T_H
#define KC_SERVER_T_H

#include "../system/logger.h"
#include "socket.h"

#include <stdio.h>
#include <stdbool.h>

//---------------------------------------------------------------------------//

struct kc_server_t
{
  int fd;                      // the file descriptor
  struct sockaddr_in* addr;    // socket address
  char* ip;                    // server IP address

  int (*start)   (struct kc_server_t* self);
  int (*stop)    (struct kc_server_t* self);
  int (*listen)  (int server_fd);

  void* (*dispatch)  (void* socket_fd);
};

struct kc_server_t* new_server      (const char* IP, const int PORT);
void                destroy_server  (struct kc_server_t* server);

//---------------------------------------------------------------------------//

#endif /* KC_SERVER_T_H */
