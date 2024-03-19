// This file is part of keepcoding_core
// ==================================
//
// uuid.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../../hdrs/security/uuid.h"
#include "../../hdrs/security/sysdep.h"

#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int uuid_create_ver_1  (struct kc_uuid_t* self);
int uuid_create_ver_3  (struct kc_uuid_t* self, struct kc_uuid_t nsid, void* name, int name_len);
int uuid_create_ver_5  (struct kc_uuid_t* self, struct kc_uuid_t nsid, void* name, int name_len);
int uuid_get_hash      (struct kc_uuid_t* self, unsigned char str_uuid[KC_UUID_LENGTH]);
int uuid_compare       (struct kc_uuid_t* self, struct kc_uuid_t* uuid);

//--- MARK: PRIVATE FUNCTION PROTOTYPES --------------------------------------//

static int _read_state         (unsigned short* clockseq, kc_uuid_time_t* timestamp, struct kc_uuid_node_t* node);
static int _write_state        (unsigned short clockseq, kc_uuid_time_t timestamp, struct kc_uuid_node_t node);
static int _format_uuid_v1     (struct kc_uuid_t* uuid, unsigned short clockseq, kc_uuid_time_t timestamp, struct kc_uuid_node_t node);
static int _format_uuid_v3or5  (struct kc_uuid_t* uuid, unsigned char hash[16], int v);
static int _get_current_time   (kc_uuid_time_t* timestamp);

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

struct kc_uuid_t* new_uuid()
{
  // create a UUID instance to be returned
  struct kc_uuid_t* uuid = malloc(sizeof(struct kc_uuid_t));

  if (uuid == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // create a new logger instance
  uuid->logger = new_logger(KC_UUID_LOG_PATH);
  if (uuid->logger == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // load magic initialization constants
  uuid->time_low                  = 0x98BADCFE;
  uuid->time_mid                  = 0x6745;
  uuid->time_hi_and_version       = 0xEFCD;
  uuid->clock_seq_hi_and_reserved = 0x56;
  uuid->clock_seq_low             = 0x78;

  uuid->node[0] = 0xC3;
  uuid->node[1] = 0xB1;
  uuid->node[2] = 0x29;
  uuid->node[3] = 0xF5;
  uuid->node[4] = 0xE8;
  uuid->node[5] = 0xFA;


  // assigns the public member methods
  uuid->create_v1 = uuid_create_ver_1;
  uuid->create_v3 = uuid_create_ver_3;
  uuid->create_v5 = uuid_create_ver_5;
  uuid->get_uuid  = uuid_get_hash;
  uuid->compare   = uuid_compare;

  return uuid;
}

//---------------------------------------------------------------------------//

void destroy_uuid(struct kc_uuid_t* uuid)
{
  if (uuid == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  free(uuid);
}

//---------------------------------------------------------------------------//

int uuid_create_ver_1(struct kc_uuid_t* self)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  kc_uuid_time_t timestamp;
  kc_uuid_time_t last_time;

  unsigned short clockseq;

  struct kc_uuid_node_t node;
  struct kc_uuid_node_t last_node;

  int ret = KC_SUCCESS;

  // acquire system-wide
  // lock so we're alone
  LOCK;

  // get time, node ID, saved state from non-volatile storage
  ret = _get_current_time(&timestamp);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = get_ieee_node_identifier(&node);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = _read_state(&clockseq, &last_time, &last_node);

  // if no NV state, or if clock went backwards, or node ID
  // changed (e.g., new network card) change clockseq
  if (ret != KC_SUCCESS || memcmp(&node, &last_node, sizeof node))
  {
    clockseq = true_random();
  }
  else if (timestamp < last_time)
  {
    ++clockseq;
  }

  // save the state for next time
  ret = _write_state(clockseq, timestamp, node);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  UNLOCK;

  // stuff fields into the UUID
  ret = _format_uuid_v1(self, clockseq, timestamp, node);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  return KC_SUCCESS;
}

// create a version 3 (MD5) UUID using a "name" from a "name space"
int uuid_create_ver_3(struct kc_uuid_t* self, struct kc_uuid_t nsid, void* name, int name_len)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  struct kc_md5_t context;
  unsigned char hash[16];
  struct kc_uuid_t net_nsid;

  int ret = KC_SUCCESS;

  // put name space ID in network byte order so it hashes the same
  // no matter what endian machine we're on 
  net_nsid = nsid;
  net_nsid.time_low = htonl(net_nsid.time_low);
  net_nsid.time_mid = htons(net_nsid.time_mid);
  net_nsid.time_hi_and_version = htons(net_nsid.time_hi_and_version);

  ret = md5_init(&context);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = md5_update(&context, &net_nsid, sizeof(net_nsid));
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = md5_update(&context, name, name_len);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = md5_final(&context, hash);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  // the hash is in network byte order at this point 
  ret = _format_uuid_v3or5(self, hash, 3);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int uuid_create_ver_5(struct kc_uuid_t * self, struct kc_uuid_t nsid, void* name, int name_len)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  struct kc_sha1_t context;
  unsigned char hash[20];
  struct kc_uuid_t net_nsid;

  int ret = KC_SUCCESS;

  // put name space ID in network byte order so it hashes the same
  // no matter what endian machine we're on 
  net_nsid = nsid;
  net_nsid.time_low = htonl(net_nsid.time_low);
  net_nsid.time_mid = htons(net_nsid.time_mid);
  net_nsid.time_hi_and_version = htons(net_nsid.time_hi_and_version);

  ret = sha1_init(&context);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = sha1_update(&context, &net_nsid, sizeof net_nsid);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = sha1_update(&context, name, name_len);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = sha1_final(&context, hash);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  // the hash is in network byte order at this point 
  ret = _format_uuid_v3or5(self, hash, 5);
  if (ret != KC_SUCCESS)
  {
    self->logger->log(self->logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int uuid_get_hash(struct kc_uuid_t* self, unsigned char str_uuid[KC_UUID_LENGTH])
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  int ret = KC_SUCCESS;

  ret = sprintf(
    (char*)str_uuid, 
    "%8.8x-%4.4x-%4.4x-%2.2x%2.2x-%02x%02x%02x%02x%02x%02x", 
    self->time_low, 
    self->time_mid, 
    self->time_hi_and_version,
    self->clock_seq_hi_and_reserved, 
    self->clock_seq_low,
    (unsigned char)self->node[0],
    (unsigned char)self->node[1],
    (unsigned char)self->node[2],
    (unsigned char)self->node[3],
    (unsigned char)self->node[4],
    (unsigned char)self->node[5]
  );

  if (ret == KC_INVALID)
  {
    return KC_FORMAT_ERROR;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

// compare two UUID's "lexically" and return 
#define CHECK(f1, f2) if (f1 != f2) return f1 < f2 ? -1 : 1;

int uuid_compare(struct kc_uuid_t* u1, struct kc_uuid_t* u2)
{
  if (u1 == NULL || u2 == NULL)
  {
    return 0;
  }

  CHECK(u1->time_low, u2->time_low);
  CHECK(u1->time_mid, u2->time_mid);

  CHECK(u1->time_hi_and_version, u2->time_hi_and_version);
  CHECK(u1->clock_seq_hi_and_reserved, u2->clock_seq_hi_and_reserved);
  CHECK(u1->clock_seq_low, u2->clock_seq_low)

  for (int i = 0; i < 6; ++i)
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

static struct _uuid_state st;

int _read_state(unsigned short* clockseq, kc_uuid_time_t* timestamp, struct kc_uuid_node_t* node)
{
  static int inited = 0;
  FILE* fp;

  // only need to read state once per boot 
  if (!inited)
  {
    fp = fopen("state", "rb");

    if (fp == NULL)
    {
      return KC_FILE_NOT_FOUND;
    }

    fread(&st, sizeof st, 1, fp);
    fclose(fp);
    inited = 1;
  }

  *clockseq = st.cs;
  *timestamp = st.ts;
  *node = st.node;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _write_state(unsigned short clockseq, kc_uuid_time_t timestamp, struct kc_uuid_node_t node)
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

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _format_uuid_v1(struct kc_uuid_t* uuid, unsigned short clock_seq, kc_uuid_time_t timestamp, struct kc_uuid_node_t node)
{
  if (uuid == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // construct a version 1 uuid with the information 
  // we've gathered plus a few constants
  uuid->time_low = (unsigned long)(timestamp & 0xFFFFFFFF);
  uuid->time_mid = (unsigned short)((timestamp >> 32) & 0xFFFF);
  uuid->time_hi_and_version = (unsigned short)((timestamp >> 48) & 0x0FFF);
  uuid->time_hi_and_version |= (1 << 12);
  uuid->clock_seq_low = clock_seq & 0xFF;
  uuid->clock_seq_hi_and_reserved = (clock_seq & 0x3F00) >> 8;
  uuid->clock_seq_hi_and_reserved |= 0x80;

  void* ret = memcpy(&uuid->node, &node, sizeof(uuid->node));

  if (ret == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _format_uuid_v3or5(struct kc_uuid_t* uuid, unsigned char hash[16], int v)
{
  if (uuid == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // convert UUID to local byte order 
  void* ret = memcpy(uuid, hash, sizeof * uuid);
  if (ret == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  uuid->time_low = ntohl(uuid->time_low);
  uuid->time_mid = ntohs(uuid->time_mid);
  uuid->time_hi_and_version = ntohs(uuid->time_hi_and_version);

  // put in the variant and version bits 
  uuid->time_hi_and_version &= 0x0FFF;
  uuid->time_hi_and_version |= (v << 12);
  uuid->clock_seq_hi_and_reserved &= 0x3F;
  uuid->clock_seq_hi_and_reserved |= 0x80;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _get_current_time(kc_uuid_time_t* timestamp)
{
  // get time as 60-bit 100ns ticks since UUID epoch compensate 
  // for the fact that real clock resolution is less than 100ns

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

  return KC_SUCCESS;
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

