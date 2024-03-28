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
#include <netinet/in.h>
#include <sys/socket.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

static int start_client  (struct kc_client_t* self);
static int stop_client   (struct kc_client_t* self);

//---------------------------------------------------------------------------//

struct kc_client_t* new_client()
{
  // create a client instance to be returned
  struct kc_client_t* new_client = malloc(sizeof(struct kc_client_t));

  // confirm that there is memory to allocate
  if (new_client == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return NULL;
  }

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
