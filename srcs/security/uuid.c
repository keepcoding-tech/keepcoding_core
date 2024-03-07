// This file is part of keepcoding_core
// ==================================
//
// uuid.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/security/uuid.h"

#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int kc_generate_uuid  (char** uuid);

//--- MARK: PRIVATE FUNCTION PROTOTYPES --------------------------------------//

static int _reverse_string  (char str[32]);

//---------------------------------------------------------------------------//

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define Q_MAX_UID_LENGTH 50

int kc_generate_uuid(char** uuid)
{
  const char sz_table[33] = "123456789ABCDEFGHJKLMNPQRSTVWXYZ";

  // calculate the timestamp
  time_t u_time = 0;
  struct tm timestm;
  time(&u_time);
  localtime_r(&u_time, &timestm);
  u_time = mktime(&timestm);

  char sz_code[32];
  int i = 0;
  while (u_time > 0)
  {
    sz_code[i++] = sz_table[u_time & 0x1F];
    u_time >>= 5;
  }

  sz_code[i] = 0;
  _reverse_string(sz_code);

  // use process ID
  unsigned long pid = getpid();

  // calculate the length needed for the string
  int length = snprintf(NULL, 0, "%s%lu", sz_code, pid);
  if (length < 0)
  {
    return KC_SYSTEM_ERROR;
  }

  // allocate memory for the string
  (*uuid) = malloc(sizeof(char) * (length + 1));
  if (*uuid == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // create the string
  snprintf(*uuid, length + 1, "%s%lu", sz_code, pid);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _reverse_string(char str[32])
{
  int length = strlen(str);

  if (length <= 0 || length >= 32)
  {
    return KC_INVALID_ARGUMENT;
  }

  for (int i = 0; i < length / 2; ++i)
  {
    char temp = str[i];
    str[i] = str[length - i - 1];
    str[length - i - 1] = temp;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
