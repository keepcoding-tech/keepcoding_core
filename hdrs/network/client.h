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

#include "../logger/logger.h"

#include <stdio.h>

//---------------------------------------------------------------------------//

struct kc_client_t
{
  int (*start)  (struct kc_client_t* self);
  int (*stop)   (struct kc_client_t* self);
};

struct kc_client_t* new_client      ();
void                destroy_client  (struct kc_client_t* client);

//---------------------------------------------------------------------------//

#endif /* KC_CLIENT_T_H */
