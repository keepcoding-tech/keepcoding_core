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
#include "socket.h"

#include <stdio.h>
#include <stdbool.h>

//---------------------------------------------------------------------------//

struct kc_client_t
{
  struct kc_socket_t* socket;

  int (*start)  (struct kc_client_t* self);
};

struct kc_client_t* new_client_IPv4  (const char* IP, const int PORT);
struct kc_client_t* new_client_IPv6  (const char* IP, const int PORT);
struct kc_client_t* new_client       (const int AF, const char* IP, const int PORT);
void                destroy_client   (struct kc_client_t* client);

//---------------------------------------------------------------------------//

#endif /* KC_CLIENT_T_H */
