// This file is part of keepcoding_core
// ==================================
//
// uuid.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../../hdrs/security/uuid.h"

#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int uuid_create(struct kc_uuid_t* uuid);
void uuid_create_md5_from_name(struct kc_uuid_t* uuid, struct kc_uuid_t nsid, void* name, int name_len);
void uuid_create_sha1_from_name(struct kc_uuid_t* uuid, struct kc_uuid_t nsid, void* name, int name_len);
int uuid_compare(struct kc_uuid_t* u1, struct kc_uuid_t* u2);

//--- MARK: PRIVATE FUNCTION PROTOTYPES --------------------------------------//

static int read_state(unsigned short* clockseq, kc_uuid_time_t* timestamp, struct kc_uuid_node_t* node);
static void write_state(unsigned short clockseq, kc_uuid_time_t timestamp, struct kc_uuid_node_t node);
static void format_uuid_v1(struct kc_uuid_t* uuid, unsigned short clockseq, kc_uuid_time_t timestamp, struct kc_uuid_node_t node);
static void format_uuid_v3or5(struct kc_uuid_t* uuid, unsigned char hash[16], int v);
static void get_current_time(kc_uuid_time_t* timestamp);
static unsigned short true_random(void);

//---------------------------------------------------------------------------//

// data type for UUID generator persistent state 
struct _uuid_state
{
  kc_uuid_time_t        ts;       // saved timestamp 
  struct kc_uuid_node_t node;     // saved node ID 
  unsigned short        cs;       // saved clock sequence 
};

//---------------------------------------------------------------------------//

// uuid_create -- generator a UUID 
int uuid_create(struct kc_uuid_t* uuid)
{
  kc_uuid_time_t timestamp, last_time;
  unsigned short clockseq;
  struct kc_uuid_node_t node;
  struct kc_uuid_node_t last_node;
  int f;

  // acquire system-wide lock so we're alone
  LOCK;

  // get time, node ID, saved state from non-volatile storage
  get_current_time(&timestamp);
  get_ieee_node_identifier(&node);
  f = read_state(&clockseq, &last_time, &last_node);

  // if no NV state, or if clock went backwards, or node ID
  // changed (e.g., new network card) change clockseq
  if (!f || memcmp(&node, &last_node, sizeof node))
  {
    clockseq = true_random();
  }
  else if (timestamp < last_time)
  {
    clockseq++;
  }

  // save the state for next time
  write_state(clockseq, timestamp, node);

  UNLOCK;

  // stuff fields into the UUID
  format_uuid_v1(uuid, clockseq, timestamp, node);

  return 1;
}

//---------------------------------------------------------------------------//

void format_uuid_v1(struct kc_uuid_t* uuid, unsigned short clock_seq, kc_uuid_time_t timestamp, struct kc_uuid_node_t node)
{
  // construct a version 1 uuid with the information 
  // we've gathered plus a few constants
  uuid->time_low = (unsigned long)(timestamp & 0xFFFFFFFF);
  uuid->time_mid = (unsigned short)((timestamp >> 32) & 0xFFFF);
  uuid->time_hi_and_version = (unsigned short)((timestamp >> 48) & 0x0FFF);
  uuid->time_hi_and_version |= (1 << 12);
  uuid->clock_seq_low = clock_seq & 0xFF;
  uuid->clock_seq_hi_and_reserved = (clock_seq & 0x3F00) >> 8;
  uuid->clock_seq_hi_and_reserved |= 0x80;
  memcpy(&uuid->node, &node, sizeof uuid->node);
}

//---------------------------------------------------------------------------//

static struct _uuid_state st;

int read_state(unsigned short* clockseq, kc_uuid_time_t* timestamp, struct kc_uuid_node_t* node)
{
  static int inited = 0;
  FILE* fp;

  // only need to read state once per boot 
  if (!inited)
  {
    fp = fopen("state", "rb");

    if (fp == NULL)
    {
      return 0;
    }

    fread(&st, sizeof st, 1, fp);
    fclose(fp);
    inited = 1;
  }

  *clockseq = st.cs;
  *timestamp = st.ts;
  *node = st.node;

  return 1;
}

//---------------------------------------------------------------------------//

void write_state(unsigned short clockseq, kc_uuid_time_t timestamp, struct kc_uuid_node_t node)
{
  static int inited = 0;
  static kc_uuid_time_t next_save;
  FILE* fp;

  if (!inited)
  {
    next_save = timestamp;
    inited = 1;
  }

  // always save state to volatile shared state 
  st.cs = clockseq;
  st.ts = timestamp;
  st.node = node;

  if (timestamp >= next_save)
  {
    fp = fopen("state", "wb");
    fwrite(&st, sizeof st, 1, fp);
    fclose(fp);

    // schedule next save for 10 seconds from now 
    next_save = timestamp + (10 * 10 * 1000 * 1000);
  }
}

//---------------------------------------------------------------------------//

// get time as 60-bit 100ns ticks since UUID epoch 
// compensate for the fact that real clock resolution is less than 100ns. 
void get_current_time(kc_uuid_time_t* timestamp)
{
  static int inited = 0;
  static kc_uuid_time_t time_last;
  static unsigned16 uuids_this_tick;
  kc_uuid_time_t time_now;

  if (!inited)
  {
    get_system_time(&time_now);
    uuids_this_tick = UUIDS_PER_TICK;
    inited = 1;
  }

  for ( ; ; )
  {
    get_system_time(&time_now);

    // if clock reading changed since last UUID generated, 
    if (time_last != time_now)
    {
      // reset count of uuids gen'd with this clock reading 
      uuids_this_tick = 0;
      time_last = time_now;
      break;
    }

    if (uuids_this_tick < UUIDS_PER_TICK)
    {
      uuids_this_tick++;
      break;
    }
    // going too fast for our clock; spin 
  }

  // add the count of uuids to low order bits of the clock reading 
  *timestamp = time_now + uuids_this_tick;
}

//---------------------------------------------------------------------------//

// true_random -- generate a crypto-quality random number.
//**This sample doesn't do that.** 
static unsigned16 true_random(void)
{
  static int inited = 0;
  kc_uuid_time_t time_now;

  if (!inited)
  {
    get_system_time(&time_now);
    time_now = time_now / UUIDS_PER_TICK;
    srand((unsigned int)(((time_now >> 32) ^ time_now) & 0xffffffff));
    inited = 1;
  }

  return rand();
}

//---------------------------------------------------------------------------//

// create a version 3 (MD5) UUID using a "name" from a "name space"
void uuid_create_md5_from_name(struct kc_uuid_t* uuid, struct kc_uuid_t nsid, void* name, int name_len)
{
  struct kc_md5_t c;
  unsigned char hash[16];
  struct kc_uuid_t net_nsid;

  // put name space ID in network byte order so it hashes the same
  // no matter what endian machine we're on 
  net_nsid = nsid;
  net_nsid.time_low = htonl(net_nsid.time_low);
  net_nsid.time_mid = htons(net_nsid.time_mid);
  net_nsid.time_hi_and_version = htons(net_nsid.time_hi_and_version);

  md5_init(&c);
  md5_update(&c, &net_nsid, sizeof net_nsid);
  md5_update(&c, name, name_len);
  md5_final(hash, &c);

  // the hash is in network byte order at this point 
  format_uuid_v3or5(uuid, hash, 3);
}

//---------------------------------------------------------------------------//

void uuid_create_sha1_from_name(struct kc_uuid_t * uuid, struct kc_uuid_t nsid, void* name, int name_len)
{
  struct kc_sha1_t c;
  unsigned char hash[20];
  struct kc_uuid_t net_nsid;

  // put name space ID in network byte order so it hashes the same
  // no matter what endian machine we're on 
  net_nsid = nsid;
  net_nsid.time_low = htonl(net_nsid.time_low);
  net_nsid.time_mid = htons(net_nsid.time_mid);
  net_nsid.time_hi_and_version = htons(net_nsid.time_hi_and_version);

  sha1_init(&c);
  sha1_update(&c, &net_nsid, sizeof net_nsid);
  sha1_update(&c, name, name_len);
  sha1_final(hash, &c);

  // the hash is in network byte order at this point 
  format_uuid_v3or5(uuid, hash, 5);
}

//---------------------------------------------------------------------------//

void format_uuid_v3or5(struct kc_uuid_t* uuid, unsigned char hash[16], int v)
{
  // convert UUID to local byte order 
  memcpy(uuid, hash, sizeof * uuid);
  uuid->time_low = ntohl(uuid->time_low);
  uuid->time_mid = ntohs(uuid->time_mid);
  uuid->time_hi_and_version = ntohs(uuid->time_hi_and_version);

  // put in the variant and version bits 
  uuid->time_hi_and_version &= 0x0FFF;
  uuid->time_hi_and_version |= (v << 12);
  uuid->clock_seq_hi_and_reserved &= 0x3F;
  uuid->clock_seq_hi_and_reserved |= 0x80;
}

//---------------------------------------------------------------------------//

// compare two UUID's "lexically" and return 
#define CHECK(f1, f2) if (f1 != f2) return f1 < f2 ? -1 : 1;

int uuid_compare(struct kc_uuid_t* u1, struct kc_uuid_t* u2)
{
  int i;

  CHECK(u1->time_low, u2->time_low);
  CHECK(u1->time_mid, u2->time_mid);

  CHECK(u1->time_hi_and_version, u2->time_hi_and_version);
  CHECK(u1->clock_seq_hi_and_reserved, u2->clock_seq_hi_and_reserved);
  CHECK(u1->clock_seq_low, u2->clock_seq_low)

  for (i = 0; i < 6; i++)
  {
    if (u1->node[i] < u2->node[i])
    {
      return -1;
    }

    if (u1->node[i] > u2->node[i])
    {
      return 1;
    }
  }

  return 0;
}

#undef CHECK

//---------------------------------------------------------------------------//
























//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

//int kc_generate_uuid  (char** uuid);

//--- MARK: PRIVATE FUNCTION PROTOTYPES --------------------------------------//

static int _reverse_string  (char str[32]);

//---------------------------------------------------------------------------//

#include <stdio.h>
#include <time.h>
//#include <unistd.h>

#define Q_MAX_UID_LENGTH 50

//int kc_generate_uuid(char** uuid)
//{
//  const char sz_table[33] = "123456789ABCDEFGHJKLMNPQRSTVWXYZ";
//
//  // calculate the timestamp
//  time_t u_time = 0;
//  struct tm timestm;
//  time(&u_time);
//  localtime_r(&u_time, &timestm);
//  u_time = mktime(&timestm);
//
//  char sz_code[32];
//  int i = 0;
//  while (u_time > 0)
//  {
//    sz_code[i++] = sz_table[u_time & 0x1F];
//    u_time >>= 5;
//  }
//
//  sz_code[i] = 0;
//  _reverse_string(sz_code);
//
//  // use process ID
//  unsigned long pid = getpid();
//
//  // calculate the length needed for the string
//  int length = snprintf(NULL, 0, "%s%lu", sz_code, pid);
//  if (length < 0)
//  {
//    return KC_SYSTEM_ERROR;
//  }
//
//  // allocate memory for the string
//  (*uuid) = malloc(sizeof(char) * (length + 1));
//  if (*uuid == NULL)
//  {
//    return KC_OUT_OF_MEMORY;
//  }
//
//  // create the string
//  snprintf(*uuid, length + 1, "%s%lu", sz_code, pid);
//
//  return KC_SUCCESS;
//}

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
