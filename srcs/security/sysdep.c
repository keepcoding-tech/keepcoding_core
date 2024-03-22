// This file is part of keepcoding_core
// ==================================
//
// sysdep.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../../hdrs/security/sysdep.h"
#include "../../hdrs/security/md5.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int  get_ieee_node_identifier  (struct kc_uuid_node_t* node);
void get_system_time           (kc_uuid_time_t* uuid_time);
void get_random_info           (char seed[16]);

//---------------------------------------------------------------------------//

int get_ieee_node_identifier(struct kc_uuid_node_t* node)
{
  // system dependent call to get IEEE node ID,
  // which will generate a random node ID

  if (node == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  static struct kc_uuid_node_t saved_node;
  static int inited = 0;

  if (!inited)
  {
    FILE* fp;
    fp = fopen("nodeid", "rb");

    if (fp != NULL)
    {
      fread(&saved_node, sizeof saved_node, 1, fp);
      fclose(fp);
    }
    else
    {
      char seed[16];
      get_random_info(seed);
      seed[0] |= 0x01;
      memcpy(&saved_node, seed, sizeof saved_node);

      fp = fopen("nodeid", "wb");

      if (fp)
      {
        fwrite(&saved_node, sizeof saved_node, 1, fp);
        fclose(fp);
      }
    }

    inited = 1;
  }

  *node = saved_node;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

void get_system_time(kc_uuid_time_t* uuid_time)
{
  // system dependent call to get the current system time. Returned as
  // 100ns ticks since UUID epoch, but resolution may be less than 100ns

#ifdef _WIN32
  ULARGE_INTEGER time;

  // NT keeps time in FILETIME format which is 100ns ticks since
  // Jan 1, 1601. UUIDs use time in 100ns ticks since Oct 15, 1582.
  // The difference is 17 Days in Oct + 30 (Nov) + 31 (Dec)
  // + 18 years and 5 leap days. */
  GetSystemTimeAsFileTime((FILETIME*)&time);
  time.QuadPart +=
    (unsigned __int64)(1000 * 1000 * 10)               // seconds
    * (unsigned __int64)(60 * 60 * 24)                 // days
    * (unsigned __int64)(17 + 30 + 31 + 365 * 18 + 5); // # of days

  *uuid_time = time.QuadPart;

#else

  struct timeval tp;

  gettimeofday(&tp, (struct timezone*)0);

  // Offset between UUID formatted times and Unix formatted times.
  // UUID UTC base time is October 15, 1582.
  // Unix base time is January 1, 1970
  *uuid_time = ((unsigned64_t)tp.tv_sec * 10000000)
    + ((unsigned64_t)tp.tv_usec * 10)
    + I64(0x01B21DD213814000);
#endif
}

//---------------------------------------------------------------------------//

/* !! Sample code, not for use in production; see RFC 1750 !! */
void get_random_info(char seed[16])
{
#ifdef _WIN32
  struct kc_md5_t c;

  struct {
    MEMORYSTATUS m;
    SYSTEM_INFO s;
    FILETIME t;
    LARGE_INTEGER pc;
    DWORD tc;
    DWORD l;
    char hostname[MAX_COMPUTERNAME_LENGTH + 1];
  } r;

  md5_init(&c);
  GlobalMemoryStatusEx(&r.m);
  GetSystemInfo(&r.s);
  GetSystemTimeAsFileTime(&r.t);
  QueryPerformanceCounter(&r.pc);

  r.tc = GetTickCount64();
  r.l = MAX_COMPUTERNAME_LENGTH + 1;
  GetComputerNameA(r.hostname, &r.l);

  md5_update(&c, &r, sizeof r);
  md5_final(&c, seed);

#else

  struct kc_md5_t c;

  struct {
    struct sysinfo s;
    struct timeval t;
    char hostname[257];
  } r;

  md5_init(&c);
  sysinfo(&r.s);
  gettimeofday(&r.t, (struct timezone*)0);
  gethostname(r.hostname, 256);

  md5_update(&c, &r, sizeof r);
  md5_final(&c, seed);
#endif
}

//---------------------------------------------------------------------------//
