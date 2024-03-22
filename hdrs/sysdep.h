// This file is part of keepcoding_core
// ==================================
//
// sysdep.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#ifndef KC_SYSDEP_H
#define KC_SYSDEP_H

#ifdef _WIN32
#else
#endif

#ifdef _WIN32
#include <windows.h>
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>
#endif

#include <stdio.h>

#include "common.h"
#include "security/uuid.h"

//---------------------------------------------------------------------------//

// set the following to a calls to get and release a global lock
#define LOCK
#define UNLOCK

//---------------------------------------------------------------------------//

// set this to what your compiler uses for 64-bit data type
#ifdef WININC
#define unsigned64_t unsigned __int64
#define I64(C) C
#else
#define unsigned64_t unsigned long long
#define I64(C) C##LL
#endif

//---------------------------------------------------------------------------//

typedef unsigned64_t kc_uuid_time_t;

struct kc_uuid_node_t
{
  char node_id[6];
};

//---------------------------------------------------------------------------//

int  get_ieee_node_identifier  (struct kc_uuid_node_t* node);
void get_system_time           (kc_uuid_time_t* uuid_time);
void get_random_info           (char seed[16]);

//---------------------------------------------------------------------------//

#endif /* KC_SYSDEP_H */
