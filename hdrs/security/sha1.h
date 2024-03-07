// This file is part of keepcoding_core
// ==================================
//
// sha1.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * This is the header file for code which implements the Secure
 * Hashing Algorithm 1 as defined in FIPS PUB 180-1 published
 * April 17, 1995.
 *
 * Many of the variable names in this code, especially the
 * single character names, were used because those were the names
 * used in the publication.
*/

#ifndef KC_SHA1_T_H
#define KC_SHA1_T_H

#include "../common.h"

#include <stdint.h>

//---------------------------------------------------------------------------//

/*
 * If you do not have the ISO standard stdint.h header file,
 * then you must typdef the following:
 *
 *  name             meaning
 *  uint32_t         unsigned 32 bit integer
 *  uint8_t          unsigned 8 bit integer (i.e., unsigned char)
 *  int_least16_t    integer of >= 16 bits
 */

#ifndef _SHA_enum_
#define _SHA_enum_

enum
{
  shaSuccess = 0,
  shaNull,            /* Null pointer parameter */
  shaInputTooLong,    /* input data too long */
  shaStateError       /* called Input after Result */
};

#endif

//---------------------------------------------------------------------------//

#define SHA1_HASH_SIZE 20

// this structure will hold context information
// for the SHA-1 hashing operation
struct kc_sha1_t
{
  // message digest
  uint32_t intermediate_hash[SHA1_HASH_SIZE / 4];

  // message length in bits
  uint32_t length_low;
  uint32_t length_high;

  // index into message block array
  int_least16_t message_block_index;

  // 512-bit message blocks
  uint8_t message_block[64];

  int computed;   // Is the digest computed?
  int corrupted;  // Is the message digest corrupted? 
};

//---------------------------------------------------------------------------//

int sha1_init   (struct kc_sha1_t* context);
int sha1_update  (struct kc_sha1_t* context, const uint8_t* msg_array, unsigned int len);
int sha1_final (struct kc_sha1_t* context, uint8_t msg_digest[SHA1_HASH_SIZE]);

//---------------------------------------------------------------------------//

// SHA1 circular left shift macro
#define SHA1_CIRCULAR_SHIFT(bits, word)           \
  (((word) << (bits)) | ((word) >> (32-(bits))))

//---------------------------------------------------------------------------//

#endif /* KC_SHA1_T_H */
