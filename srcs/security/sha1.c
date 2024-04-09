// This file is part of keepcoding_core
// ==================================
//
// sha1.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../../hdrs/security/sha1.h"

#include <stdlib.h>
#include <string.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int sha1_init    (struct kc_sha1_t* sha1);
int sha1_update  (struct kc_sha1_t* sha1, const uint8_t* msg_arrey, unsigned int len);
int sha1_final   (struct kc_sha1_t* sha1, uint8_t msg_digest[KC_SHA1_LENGTH]);

int sha1_to_string  (uint8_t digest[KC_SHA1_LENGTH], unsigned char str_hash[(KC_SHA1_LENGTH * 2) + 1]);

//--- MARK: PRIVATE FUNCTION PROTOTYPES --------------------------------------//

int _sha1_process_message_block  (struct kc_sha1_t* sha1);
int _sha1_pad_message            (struct kc_sha1_t* sha1);

//---------------------------------------------------------------------------//

struct kc_sha1_t* new_sha1(void)
{
  // create a SHA1 instance to be returned
  struct kc_sha1_t* new_sha1 = malloc(sizeof(struct kc_sha1_t));

  // confirm that there is memory to allocate
  if (new_sha1 == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // initialize the structure members fields
  int ret = sha1_init(new_sha1);
  if (ret != KC_SUCCESS)
  {
    return NULL;
  }

  // assigns the public member methods
  new_sha1->digest   = sha1_update;
  new_sha1->get_hash = sha1_final;

  return new_sha1;
}

//---------------------------------------------------------------------------//

void destroy_sha1(struct kc_sha1_t* sha1)
{
  if (sha1 == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(sha1);
}

//---------------------------------------------------------------------------//

int sha1_init(struct kc_sha1_t* sha1)
{
  if (sha1 == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // create a new logger instance
  sha1->_logger = new_logger(KC_SHA1_LOG_PATH);
  if (sha1->_logger == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return KC_OUT_OF_MEMORY;
  }

  sha1->length_low = 0;
  sha1->length_high = 0;
  sha1->message_block_index = 0;

  // load magic initialization constants
  sha1->intermediate_hash[0] = 0x67452301;
  sha1->intermediate_hash[1] = 0xEFCDAB89;
  sha1->intermediate_hash[2] = 0x98BADCFE;
  sha1->intermediate_hash[3] = 0x10325476;
  sha1->intermediate_hash[4] = 0xC3D2E1F0;

  sha1->computed = 0;
  sha1->corrupted = 0;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int sha1_update(struct kc_sha1_t* sha1, const uint8_t* msg_array, unsigned int len)
{
  if (len == 0)
  {
    return KC_SUCCESS;
  }

  if (sha1 == NULL || msg_array == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  if (sha1->computed)
  {
    sha1->corrupted = KC_SHA1_STATE_ERROR;
    return KC_SHA1_STATE_ERROR;
  }

  if (sha1->corrupted)
  {
    return sha1->corrupted;
  }

  int ret = KC_SUCCESS;

  while (len-- && !sha1->corrupted)
  {
    sha1->message_block[sha1->message_block_index++] = (*msg_array & 0xFF);

    sha1->length_low += 8;

    if (sha1->length_low == 0)
    {
      sha1->length_high++;
      if (sha1->length_high == 0)
      {
        // message is too long
        sha1->corrupted = KC_SHA1_INPUT_TOO_LONG;
      }
    }

    if (sha1->message_block_index == 64)
    {
      ret = _sha1_process_message_block(sha1);
      if (ret != KC_SUCCESS)
      {
        sha1->_logger->log(sha1->_logger, KC_WARNING_LOG, ret,
          __FILE__, __LINE__, __func__);
        return ret;
      }
    }

    msg_array++;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int sha1_final(struct kc_sha1_t* sha1, uint8_t msg_digest[KC_SHA1_LENGTH])
{
  if (sha1 == NULL || msg_digest == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  int ret = KC_SUCCESS;

  if (sha1->corrupted)
  {
    return sha1->corrupted;
  }

  if (!sha1->computed)
  {
    ret = _sha1_pad_message(sha1);
    if (ret != KC_SUCCESS)
    {
      sha1->_logger->log(sha1->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
      return ret;
    }

    // message may be sensitive, clear it out
    for (int i = 0; i < 64; ++i)
    {
      sha1->message_block[i] = 0;
    }

    // and clear length
    sha1->length_low = 0;
    sha1->length_high = 0;
    sha1->computed = 1;
  }

  for (int i = 0; i < KC_SHA1_LENGTH; ++i)
  {
    msg_digest[i] = sha1->intermediate_hash[i >> 2] >> 8 * (3 - (i & 0x03));
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int sha1_to_string(uint8_t digest[KC_SHA1_LENGTH], unsigned char str_hash[(KC_SHA1_LENGTH * 2) + 1])
{
  int ret = KC_SUCCESS;

  ret = sprintf(
    (char*)str_hash,
    "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
    "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
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
    digest[15],
    digest[16],
    digest[17],
    digest[18],
    digest[19]
  );

  if (ret == KC_INVALID)
  {
    return KC_FORMAT_ERROR;
  }


  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _sha1_process_message_block(struct kc_sha1_t* sha1)
{
  if (sha1 == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // Constants defined in SHA-1
  const uint32_t K[] =
  {
    0x5A827999,
    0x6ED9EBA1,
    0x8F1BBCDC,
    0xCA62C1D6
  };

  int      t;                 // Loop counter
  uint32_t temp;              // Temporary 32 value
  uint32_t W[80];             // 32 sequence
  uint32_t A, B, C, D, E;     // 32 buffers

  // initialize the first 16 words in the array W
  for (t = 0; t < 16; ++t)
  {
    W[t] = sha1->message_block[t * 4] << 24;
    W[t] |= sha1->message_block[t * 4 + 1] << 16;
    W[t] |= sha1->message_block[t * 4 + 2] << 8;
    W[t] |= sha1->message_block[t * 4 + 3];
  }

  for (t = 16; t < 80; ++t)
  {
    W[t] = SHA1_CIRCULAR_SHIFT(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
  }

  A = sha1->intermediate_hash[0];
  B = sha1->intermediate_hash[1];
  C = sha1->intermediate_hash[2];
  D = sha1->intermediate_hash[3];
  E = sha1->intermediate_hash[4];

  for (t = 0; t < 20; ++t)
  {
    temp = SHA1_CIRCULAR_SHIFT(5, A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
    E = D;
    D = C;
    C = SHA1_CIRCULAR_SHIFT(30, B);
    B = A;
    A = temp;
  }

  for (t = 20; t < 40; ++t)
  {
    temp = SHA1_CIRCULAR_SHIFT(5, A) + (B ^ C ^ D) + E + W[t] + K[1];
    E = D;
    D = C;
    C = SHA1_CIRCULAR_SHIFT(30, B);
    B = A;
    A = temp;
  }

  for (t = 40; t < 60; ++t)
  {
    temp = SHA1_CIRCULAR_SHIFT(5, A) +
      ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
    E = D;
    D = C;
    C = SHA1_CIRCULAR_SHIFT(30, B);
    B = A;
    A = temp;
  }

  for (t = 60; t < 80; ++t)
  {
    temp = SHA1_CIRCULAR_SHIFT(5, A) + (B ^ C ^ D) + E + W[t] + K[3];
    E = D;
    D = C;
    C = SHA1_CIRCULAR_SHIFT(30, B);
    B = A;
    A = temp;
  }

  sha1->intermediate_hash[0] += A;
  sha1->intermediate_hash[1] += B;
  sha1->intermediate_hash[2] += C;
  sha1->intermediate_hash[3] += D;
  sha1->intermediate_hash[4] += E;

  sha1->message_block_index = 0;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _sha1_pad_message(struct kc_sha1_t* sha1)
{
  if (sha1 == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  int ret = KC_SUCCESS;

  // check to see if the current message block is too small to hold
  // the initial padding bits and length
  //
  // if so, we will pad the block, process it, and then continue
  // padding into a second block

  if (sha1->message_block_index > 55)
  {
    sha1->message_block[sha1->message_block_index++] = 0x80;

    while (sha1->message_block_index < 64)
    {
      sha1->message_block[sha1->message_block_index++] = 0;
    }

    ret = _sha1_process_message_block(sha1);
    if (ret != KC_SUCCESS)
    {
      sha1->_logger->log(sha1->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
      return ret;
    }

    while (sha1->message_block_index < 56)
    {
      sha1->message_block[sha1->message_block_index++] = 0;
    }
  }
  else
  {
    sha1->message_block[sha1->message_block_index++] = 0x80;

    while (sha1->message_block_index < 56)
    {
      sha1->message_block[sha1->message_block_index++] = 0;
    }
  }

  // store the message length as the last 8 octets
  sha1->message_block[56] = sha1->length_high >> 24;
  sha1->message_block[57] = sha1->length_high >> 16;
  sha1->message_block[58] = sha1->length_high >> 8;
  sha1->message_block[59] = sha1->length_high;

  sha1->message_block[60] = sha1->length_low >> 24;
  sha1->message_block[61] = sha1->length_low >> 16;
  sha1->message_block[62] = sha1->length_low >> 8;
  sha1->message_block[63] = sha1->length_low;

  ret = _sha1_process_message_block(sha1);
  if (ret != KC_SUCCESS)
  {
    sha1->_logger->log(sha1->_logger, KC_WARNING_LOG, ret,
      __FILE__, __LINE__, __func__);
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

