// This file is part of keepcoding_core
// ==================================
//
// client.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * a network struct
 */

#ifndef KC_CLIENT_T_H
#define KC_CLIENT_T_H

#include "../system/logger.h"

#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>

//---------------------------------------------------------------------------//

struct kc_client_t
{
  // the file descriptor
  int fd;

  // socket address
  struct sockaddr_in* addr;

  int (*start)  (struct kc_client_t* self);
  int (*stop)   (struct kc_client_t* self);
};

struct kc_client_t* new_client      (const char* IP, const int PORT);
void                destroy_client  (struct kc_client_t* client);

//---------------------------------------------------------------------------//

#endif /* KC_CLIENT_T_H */
