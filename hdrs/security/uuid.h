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

#include "../common.h"
#include "md5.h"
#include "sha1.h"

#include <stdio.h>

/*
 *
 */

//----------------- sysdep.h ----------------- //

typedef unsigned long long kc_uuid_time_t;

struct kc_uuid_node_t
{
  char nodeID[6];
};

#define LOCK
#define UNLOCK
#define UUIDS_PER_TICK 1024

typedef unsigned long   unsigned32;
typedef unsigned short  unsigned16;
typedef unsigned char   unsigned8;
typedef unsigned char   byte;

//---------------------------------------------------------------------------//

struct kc_uuid_t
{
  unsigned long  time_low;
  unsigned short time_mid;
  unsigned short time_hi_and_version;
  unsigned char  clock_seq_hi_and_reserved;
  unsigned char  clock_seq_low;
  char           node[6];
};

/* uuid_create -- generate a UUID */
int uuid_create(struct kc_uuid_t * uuid);

/* uuid_create_md5_from_name -- create a version 3 (MD5) UUID using a
   "name" from a "name space" */
void uuid_create_md5_from_name(struct kc_uuid_t* uuid, struct kc_uuid_t nsid, void* name, int name_len);

/* uuid_create_sha1_from_name -- create a version 5 (SHA-1) UUID
   using a "name" from a "name space" */
void uuid_create_sha1_from_name(struct kc_uuid_t* uuid, struct kc_uuid_t nsid, void* name, int name_len);

/* uuid_compare --  Compare two UUID's "lexically" and return
        -1   u1 is lexically before u2
         0   u1 is equal to u2
         1   u1 is lexically after u2
   Note that lexical ordering is not temporal ordering!
*/
int uuid_compare(struct kc_uuid_t* u1, struct kc_uuid_t* u2);

//---------------------------------------------------------------------------//

int kc_generate_uuid  (char** uuid);

//---------------------------------------------------------------------------//

#endif /* KC_UUID_T_H */
