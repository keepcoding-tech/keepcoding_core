// This file is part of keepcoding_core
// ==================================
//
// md5.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License
//
// RSA Data Security, Inc. MD5 Message-Digest Algorithm

#define _CRT_SECURE_NO_WARNINGS

#include "../../hdrs/security/md5.h"

#include <stdlib.h>
#include <string.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int md5_init   (struct kc_md5_t* md5);
int md5_update (struct kc_md5_t* md5, unsigned char* input, unsigned int in_len);
int md5_final  (struct kc_md5_t* md5, unsigned char digest[(KC_MD5_LENGTH * 2) + 1]);

int md5_to_string(unsigned char digest[KC_MD5_LENGTH], unsigned char str_hash[(KC_MD5_LENGTH * 2) + 1]);

//--- MARK: PRIVATE FUNCTION PROTOTYPES --------------------------------------//

static int _md5_transform (UINT4[4], unsigned char[64]);
static int _encode        (unsigned char*, UINT4*, unsigned int);
static int _decode        (UINT4*, unsigned char*, unsigned int);
static int _md5_memcpy    (POINTER, POINTER, unsigned int);
static int _md5_memset    (POINTER, int, unsigned int);

//---------------------------------------------------------------------------//

static unsigned char PADDING[64] =
{
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//---------------------------------------------------------------------------//

struct kc_md5_t* new_md5(void)
{
  // create a MD5 instance to be returned
  struct kc_md5_t* new_md5 = malloc(sizeof(struct kc_md5_t));

  // confirm that there is memory to allocate
  if (new_md5 == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // initialize the structure members fields
  int ret = md5_init(new_md5);
  if (ret != KC_SUCCESS)
  {
    if (ret == KC_NULL_REFERENCE)
    {
      log_error(KC_NULL_REFERENCE_LOG);
    }
    else if (ret == KC_OUT_OF_MEMORY)
    {
      log_error(KC_OUT_OF_MEMORY_LOG);
    }

    return NULL;
  }

  // assigns the public member methods
  new_md5->digest   = md5_update;
  new_md5->get_hash = md5_final;

  return new_md5;
}

//---------------------------------------------------------------------------//

void destroy_md5(struct kc_md5_t* md5)
{
  if (md5 == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(md5);
}

//---------------------------------------------------------------------------//

int md5_init(struct kc_md5_t* md5)
{
  if (md5 == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // create a new logger instance
  md5->_logger = new_logger(KC_MD5_LOG_PATH);
  if (md5->_logger == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return KC_OUT_OF_MEMORY;
  }

  md5->count[0] = md5->count[1] = 0;

  // load magic initialization constants
  md5->state[0] = 0x67452301;
  md5->state[1] = 0xefcdab89;
  md5->state[2] = 0x98badcfe;
  md5->state[3] = 0x10325476;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int md5_update(struct kc_md5_t* md5, unsigned char* input, unsigned int in_len)
{
  if (md5 == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  int ret = KC_SUCCESS;

  unsigned int i, index, part_len;

  // compute number of bytes mod 64
  index = (unsigned int)((md5->count[0] >> 3) & 0x3F);

  // update number of bits
  if ((md5->count[0] += ((UINT4)in_len << 3)) < ((UINT4)in_len << 3))
  {
    md5->count[1]++;
  }

  md5->count[1] += ((UINT4)in_len >> 29);

  part_len = 64 - index;

  // transform as many times as possible.
  if (in_len >= part_len)
  {
    ret = _md5_memcpy((POINTER)&md5->buffer[index], (POINTER)input, part_len);
    if (ret != KC_SUCCESS)
    {
      md5->_logger->log(md5->_logger, KC_WARNING_LOG, ret,
        __FILE__, __LINE__, __func__);
      return ret;
    }

    ret = _md5_transform(md5->state, md5->buffer);
    if (ret != KC_SUCCESS)
    {
      md5->_logger->log(md5->_logger, KC_WARNING_LOG, ret,
        __FILE__, __LINE__, __func__);
      return ret;
    }

    for (i = part_len; i + 63 < in_len; i += 64)
    {
      ret = _md5_transform(md5->state, &input[i]);
      if (ret != KC_SUCCESS)
      {
        md5->_logger->log(md5->_logger, KC_WARNING_LOG, ret,
          __FILE__, __LINE__, __func__);
        return ret;
      }
    }

    index = 0;
  }
  else
  {
    i = 0;
  }

  // buffer remaining input
  ret = _md5_memcpy((POINTER)&md5->buffer[index], (POINTER)&input[i], in_len - i);
  if (ret != KC_SUCCESS)
  {
    md5->_logger->log(md5->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int md5_final(struct kc_md5_t* md5, unsigned char digest[(KC_MD5_LENGTH * 2) + 1])
{
  if (md5 == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  int ret = KC_SUCCESS;

  unsigned char bits[8];
  unsigned int index, padLen;

  // save number of bits
  ret = _encode(bits, md5->count, 8);
  if (ret != KC_SUCCESS)
  {
    md5->_logger->log(md5->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  // pad out to 56 mod 64
  index = (unsigned int)((md5->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);

  ret = md5_update(md5, PADDING, padLen);
  if (ret != KC_SUCCESS)
  {
    md5->_logger->log(md5->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  // append length (before padding)
  ret = md5_update(md5, bits, 8);
  if (ret != KC_SUCCESS)
  {
    md5->_logger->log(md5->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  // store state in digest
  ret = _encode(digest, md5->state, 16);
  if (ret != KC_SUCCESS)
  {
    md5->_logger->log(md5->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  // zeroize sensitive information
  ret = _md5_memset((POINTER)md5, 0, sizeof(*md5));
  if (ret != KC_SUCCESS)
  {
    md5->_logger->log(md5->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int md5_to_string(unsigned char digest[KC_MD5_LENGTH], unsigned char str_hash[(KC_MD5_LENGTH * 2) + 1])
{
  int ret = KC_SUCCESS;

  ret = sprintf(
    (char*)str_hash,
    "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
    digest[0],
    digest[1],
    digest[2],
    digest[3],
    digest[4],
    digest[5],
    digest[6],
    digest[7],
    digest[8],
    digest[9],
    digest[10],
    digest[11],
    digest[12],
    digest[13],
    digest[14],
    digest[15]
  );

  if (ret == KC_INVALID)
  {
    return KC_FORMAT_ERROR;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

static int _md5_transform(UINT4 state[4], unsigned char block[64])
{
  int ret = KC_SUCCESS;

  UINT4 a = state[0];
  UINT4 b = state[1];
  UINT4 c = state[2];
  UINT4 d = state[3];

  UINT4 x[16];

  ret = _decode(x, block, 64);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  // Round 1
  FF(a, b, c, d, x[0], S11, 0xd76aa478);  /* 1 */
  FF(d, a, b, c, x[1], S12, 0xe8c7b756);  /* 2 */
  FF(c, d, a, b, x[2], S13, 0x242070db);  /* 3 */
  FF(b, c, d, a, x[3], S14, 0xc1bdceee);  /* 4 */
  FF(a, b, c, d, x[4], S11, 0xf57c0faf);  /* 5 */
  FF(d, a, b, c, x[5], S12, 0x4787c62a);  /* 6 */
  FF(c, d, a, b, x[6], S13, 0xa8304613);  /* 7 */
  FF(b, c, d, a, x[7], S14, 0xfd469501);  /* 8 */
  FF(a, b, c, d, x[8], S11, 0x698098d8);  /* 9 */
  FF(d, a, b, c, x[9], S12, 0x8b44f7af);  /* 10 */
  FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

  // Round 2
  GG(a, b, c, d, x[1], S21, 0xf61e2562);  /* 17 */
  GG(d, a, b, c, x[6], S22, 0xc040b340);  /* 18 */
  GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);  /* 20 */
  GG(a, b, c, d, x[5], S21, 0xd62f105d);  /* 21 */
  GG(d, a, b, c, x[10], S22, 0x2441453);  /* 22 */
  GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);  /* 24 */
  GG(a, b, c, d, x[9], S21, 0x21e1cde6);  /* 25 */
  GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG(c, d, a, b, x[3], S23, 0xf4d50d87);  /* 27 */
  GG(b, c, d, a, x[8], S24, 0x455a14ed);  /* 28 */
  GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG(d, a, b, c, x[2], S22, 0xfcefa3f8);  /* 30 */
  GG(c, d, a, b, x[7], S23, 0x676f02d9);  /* 31 */
  GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  // Round 3
  HH(a, b, c, d, x[5], S31, 0xfffa3942);  /* 33 */
  HH(d, a, b, c, x[8], S32, 0x8771f681);  /* 34 */
  HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH(a, b, c, d, x[1], S31, 0xa4beea44);  /* 37 */
  HH(d, a, b, c, x[4], S32, 0x4bdecfa9);  /* 38 */
  HH(c, d, a, b, x[7], S33, 0xf6bb4b60);  /* 39 */
  HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH(d, a, b, c, x[0], S32, 0xeaa127fa);  /* 42 */
  HH(c, d, a, b, x[3], S33, 0xd4ef3085);  /* 43 */
  HH(b, c, d, a, x[6], S34, 0x4881d05);   /* 44 */
  HH(a, b, c, d, x[9], S31, 0xd9d4d039);  /* 45 */
  HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH(b, c, d, a, x[2], S34, 0xc4ac5665);  /* 48 */

  // Round 4
  II(a, b, c, d, x[0], S41, 0xf4292244);  /* 49 */
  II(d, a, b, c, x[7], S42, 0x432aff97);  /* 50 */
  II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II(b, c, d, a, x[5], S44, 0xfc93a039);  /* 52 */
  II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II(d, a, b, c, x[3], S42, 0x8f0ccc92);  /* 54 */
  II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II(b, c, d, a, x[1], S44, 0x85845dd1);  /* 56 */
  II(a, b, c, d, x[8], S41, 0x6fa87e4f);  /* 57 */
  II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II(c, d, a, b, x[6], S43, 0xa3014314);  /* 59 */
  II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II(a, b, c, d, x[4], S41, 0xf7537e82);  /* 61 */
  II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II(c, d, a, b, x[2], S43, 0x2ad7d2bb);  /* 63 */
  II(b, c, d, a, x[9], S44, 0xeb86d391);  /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  // zeroize sensitive information
  ret = _md5_memset((POINTER)x, 0, sizeof(x));
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

static int _encode(unsigned char* output, UINT4* input, unsigned int len)
{
  if (output == NULL || input == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  if (len % 4 != 0)
  {
    return KC_INVALID_ARGUMENT;
  }

  for (unsigned int i = 0, j = 0; j < len; ++i, j += 4)
  {
    output[j] = (unsigned char)(input[i] & 0xff);
    output[j + 1] = (unsigned char)((input[i] >> 8) & 0xff);
    output[j + 2] = (unsigned char)((input[i] >> 16) & 0xff);
    output[j + 3] = (unsigned char)((input[i] >> 24) & 0xff);
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

static int _decode(UINT4* output, unsigned char* input, unsigned int len)
{
  if (output == NULL || input == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  if (len % 4 != 0)
  {
    return KC_INVALID_ARGUMENT;
  }

  for (unsigned int i = 0, j = 0; j < len; ++i, j += 4)
  {
    output[i] = ((UINT4)input[j]) | (((UINT4)input[j + 1]) << 8) |
      (((UINT4)input[j + 2]) << 16) | (((UINT4)input[j + 3]) << 24);
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

static int _md5_memcpy(POINTER output, POINTER input, unsigned int len)
{
  if (output == NULL || input == NULL)
  {
    return KC_NULL_REFERENCE;
  }

#ifdef __STDC_NO_MEMORY__
  // If __STDC_NO_MEMORY__ is defined, 'memcpy' is not available
  for (unsigned int i = 0; i < len; ++i)
  {
    output[i] = input[i];
  }
#else
  void* ret = memcpy(output, input, len);
  if (ret == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }
#endif

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

static int _md5_memset(POINTER output, int value, unsigned int len)
{
  if (output == NULL)
  {
    return KC_NULL_REFERENCE;
  }

#ifdef __STDC_NO_MEMORY__
  // If __STDC_NO_MEMORY__ is defined, 'memset' is not available
  for (unsigned int i = 0; i < len; ++i)
  {
    ((char*)output)[i] = (char)value;
  }
#else
  void* ret = memset(output, value, len);
  if (ret == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }
#endif

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
