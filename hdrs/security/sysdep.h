// This file is part of keepcoding_core
// ==================================
//
// sysdep.h
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

#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#endif

#include "../common.h"
#include "md5.h"

#include <stdio.h>

// set the following to the number of 100ns ticks 
// of the actual resolution of your system's clock
#define UUIDS_PER_TICK 1024

// set the following to a calls to get and release a global lock
#define LOCK
#define UNLOCK

typedef unsigned long   unsigned32;
typedef unsigned short  unsigned16;
typedef unsigned char   unsigned8;
typedef unsigned char   byte;

// set this to what your compiler uses for 64-bit data type
#ifdef WININC
#define unsigned64_t unsigned __int64
#define I64(C) C
#else
#define unsigned64_t unsigned long long
#define I64(C) C##LL
#endif

typedef unsigned64_t kc_uuid_time_t;

struct kc_uuid_node_t
{
  char node_id[6];
};

// system dependent call to get IEEE node ID, 
// which will generate a random node ID
void get_ieee_node_identifier(struct kc_uuid_node_t* node);

// system dependent call to get the current system time. Returned as
// 100ns ticks since UUID epoch, but resolution may be less than 100ns
void get_system_time(kc_uuid_time_t* uuid_time);

/* !! Sample code, not for use in production; see RFC 1750 !! */
void get_random_info(char seed[16]);