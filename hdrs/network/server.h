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

#include "../logger/logger.h"

#include <stdio.h>

//---------------------------------------------------------------------------//

struct kc_server_t
{
  int (*start)  (struct kc_server_t* self);
  int (*stop)   (struct kc_server_t* self);
};

struct kc_server_t* new_server      ();
void                destroy_server  (struct kc_server_t* server);

//---------------------------------------------------------------------------//

#endif /* KC_SERVER_T_H */
