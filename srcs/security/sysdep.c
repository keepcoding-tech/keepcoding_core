// This file is part of keepcoding_core
// ==================================
//
// sysdep.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../../hdrs/security/sysdep.h"

#include <stdlib.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

void get_ieee_node_identifier(struct kc_uuid_node_t* node);
void get_system_time(kc_uuid_time_t* uuid_time);
void get_random_info(char seed[16]);

//---------------------------------------------------------------------------//

void get_ieee_node_identifier(struct kc_uuid_node_t* node)
{
  static struct kc_uuid_node_t saved_node;
  static inited = 0;

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
}

//---------------------------------------------------------------------------//

#ifdef _WIN32
void get_system_time(kc_uuid_time_t* uuid_time)
{
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
}

//---------------------------------------------------------------------------//

/* !! Sample code, not for use in production; see RFC 1750 !! */
void get_random_info(char seed[16])
{
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
  md5_final(seed, &c);
}

#else //---------------------------------------------------------------------//

void get_system_time(uuid_time_t* uuid_time)
{
  struct timeval tp;

  gettimeofday(&tp, (struct timezone*)0);

  // Offset between UUID formatted times and Unix formatted times.
  // UUID UTC base time is October 15, 1582.
  // Unix base time is January 1, 1970
  *uuid_time = ((unsigned64)tp.tv_sec * 10000000)
    + ((unsigned64)tp.tv_usec * 10)
    + I64(0x01B21DD213814000);
}

//---------------------------------------------------------------------------//

/* !! Sample code, not for use in production; see RFC 1750 !! */
void get_random_info(char seed[16])
{
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
  md5_final(seed, &c);
}

#endif

//---------------------------------------------------------------------------//
