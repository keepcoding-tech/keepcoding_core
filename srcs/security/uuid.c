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
#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

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

static int  _get_ieee_node_identifier  (struct kc_uuid_node_t* node);
static void _get_system_time           (kc_uuid_time_t* uuid_time);
static void _get_random_info           (char seed[16]);

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

struct kc_uuid_t* new_uuid(void)
{
  // create a UUID instance to be returned
  struct kc_uuid_t* new_uuid = malloc(sizeof(struct kc_uuid_t));

  if (new_uuid == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // create a new logger instance
  new_uuid->_logger = new_logger(KC_UUID_LOG_PATH);
  if (new_uuid->_logger == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // load magic initialization constants
  new_uuid->time_low                  = 0x98BADCFE;
  new_uuid->time_mid                  = 0x6745;
  new_uuid->time_hi_and_version       = 0xEFCD;
  new_uuid->clock_seq_hi_and_reserved = 0x56;
  new_uuid->clock_seq_low             = 0x78;

  new_uuid->node[0] = 0x61;
  new_uuid->node[1] = 0x79;
  new_uuid->node[2] = 0x11;
  new_uuid->node[3] = 0x24;
  new_uuid->node[4] = 0x06;
  new_uuid->node[5] = 0x14;


  // assigns the public member methods
  new_uuid->create_v1 = uuid_create_ver_1;
  new_uuid->create_v3 = uuid_create_ver_3;
  new_uuid->create_v5 = uuid_create_ver_5;
  new_uuid->get_uuid  = uuid_get_hash;
  new_uuid->compare   = uuid_compare;

  return new_uuid;
}

//---------------------------------------------------------------------------//

void destroy_uuid(struct kc_uuid_t* uuid)
{
  if (uuid == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return;
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
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = _get_ieee_node_identifier(&node);
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
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
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  UNLOCK;

  // stuff fields into the UUID
  ret = _format_uuid_v1(self, clockseq, timestamp, node);
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
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
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = md5_update(&context, (unsigned char*)&net_nsid, sizeof(net_nsid));
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = md5_update(&context, name, name_len);
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = md5_final(&context, hash);
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  // the hash is in network byte order at this point
  ret = _format_uuid_v3or5(self, hash, 3);
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
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
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = sha1_update(&context, (void*)&net_nsid, sizeof net_nsid);
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = sha1_update(&context, name, name_len);
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  ret = sha1_final(&context, hash);
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  // the hash is in network byte order at this point
  ret = _format_uuid_v3or5(self, hash, 5);
  if (ret != KC_SUCCESS)
  {
    self->_logger->log(self->_logger, KC_WARNING_LOG, ret,
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
    "%8.8lx-%4.4x-%4.4x-%2.2x%2.2x-%02x%02x%02x%02x%02x%02x",
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
  static unsigned short uuids_this_tick;
  kc_uuid_time_t time_now;

  if (!inited)
  {
    _get_system_time(&time_now);
    uuids_this_tick = UUIDS_PER_TICK;
    inited = 1;
  }

  for ( ; ; )
  {
    _get_system_time(&time_now);

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
static unsigned short true_random(void)
{
  static int inited = 0;
  kc_uuid_time_t time_now;

  if (!inited)
  {
    _get_system_time(&time_now);
    time_now = time_now / UUIDS_PER_TICK;
    srand((unsigned int)(((time_now >> 32) ^ time_now) & 0xffffffff));
    inited = 1;
  }

  return rand();
}

//---------------------------------------------------------------------------//

int _get_ieee_node_identifier(struct kc_uuid_node_t* node)
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
      _get_random_info(seed);
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

void _get_system_time(kc_uuid_time_t* uuid_time)
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
  *uuid_time = ((unsigned long long)tp.tv_sec * 10000000)
    + ((unsigned long long)tp.tv_usec * 10)
    + I64(0x01B21DD213814000);
#endif
}

//---------------------------------------------------------------------------//

/* !! Sample code, not for use in production; see RFC 1750 !! */
void _get_random_info(char seed[16])
{
#ifdef __APPLE__
  arc4random_buf(seed, 16);
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
