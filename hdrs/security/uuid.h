// This file is part of keepcoding_core
// ==================================
//
// uuid.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
** Copyright (c) 1990- 1993, 1996 Open Software Foundation, Inc.
** Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, Ca. &
** Digital Equipment Corporation, Maynard, Mass.
** Copyright (c) 1998 Microsoft.
** To anyone who acknowledges that this file is provided "AS IS"
** without any express or implied warranty: permission to use, copy,
** modify, and distribute this file for any purpose is hereby
** granted without fee, provided that the above copyright notices and
** this notice appears in all source code copies, and that none of
** the names of Open Software Foundation, Inc., Hewlett-Packard
** Company, Microsoft, or Digital Equipment Corporation be used in
** advertising or publicity pertaining to distribution of the software
** without specific, written prior permission. Neither Open Software
** Foundation, Inc., Hewlett-Packard Company, Microsoft, nor Digital
** Equipment Corporation makes any representations about the
** suitability of this software for any purpose.
*/

#ifndef KC_UUID_T_H
#define KC_UUID_T_H


#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include "../common.h"
#include "md5.h"
#include "sha1.h"

#include <stdio.h>

//---------------------------------------------------------------------------//

#define KC_UUID_LOG_PATH  "build/log/uuid.log"

#define KC_UUID_LENGTH 36

//---------------------------------------------------------------------------//

struct kc_uuid_t
{
  unsigned long  time_low;
  unsigned short time_mid;
  unsigned short time_hi_and_version;
  unsigned char  clock_seq_hi_and_reserved;
  unsigned char  clock_seq_low;
  char           node[6];

  struct kc_logger_t* logger;

  int (*create_v1)  (struct kc_uuid_t* self);
  int (*create_v3)  (struct kc_uuid_t* self, struct kc_uuid_t nsid, void* name, int name_len);
  int (*create_v5)  (struct kc_uuid_t* self, struct kc_uuid_t nsid, void* name, int name_len);
  int (*get_uuid)   (struct kc_uuid_t* self, unsigned char str_uuid[KC_UUID_LENGTH + 1]);
  int (*compare)    (struct kc_uuid_t* self, struct kc_uuid_t* uuid);
};

struct kc_uuid_t* new_uuid      ();
void              destroy_uuid  (struct kc_uuid_t* uuid);

//---------------------------------------------------------------------------//

int uuid_create_ver_1  (struct kc_uuid_t * uuid);
int uuid_create_ver_3  (struct kc_uuid_t* uuid, struct kc_uuid_t nsid, void* name, int name_len);
int uuid_create_ver_5  (struct kc_uuid_t* uuid, struct kc_uuid_t nsid, void* name, int name_len);
int uuid_get_hash      (struct kc_uuid_t* self, unsigned char str_uuid[KC_UUID_LENGTH + 1]);
int uuid_compare       (struct kc_uuid_t* u1, struct kc_uuid_t* u2);

//---------------------------------------------------------------------------//

#endif /* KC_UUID_T_H */
