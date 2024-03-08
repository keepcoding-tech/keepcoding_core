// This file is part of keepcoding_core
// ==================================
//
// sha1.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * Secure Hash Algorithm, SHA-1, for computing a condensed representation of a 
 * message or a data file.  When a message of any length < 2^64 bits is input,
 * the SHA-1 produces a 160-bit output called a message digest.
 * 
 * The message digest can then, for example, be input to a signature algorithm 
 * which generates or verifies the signature for the message. Signing the 
 * message digest rather than the message often improves the efficiency of the 
 * process because the message digest is usually much smaller in size than the
 * message.
 * 
 * The same hash algorithm must be used by the verifier of a digital signature 
 * as was used by the creator of the digital signature. Any change to the 
 * message in transit will, with very high probability, result in a different 
 * message digest, and the signature will fail to verify.
*/

#ifndef KC_SHA1_T_H
#define KC_SHA1_T_H

#include "../common.h"

#include <stdint.h>

//---------------------------------------------------------------------------//

#ifndef KC_SHA1_ENUM_
#define KC_SHA1_ENUM_

enum
{
  KC_SHA1_STATE_ERROR = 0xF0000001
};

#endif

//---------------------------------------------------------------------------//

#define SHA1_HASH_SIZE 20

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

int sha1_init    (struct kc_sha1_t* context);
int sha1_update  (struct kc_sha1_t* context, const uint8_t* msg_array, unsigned int len);
int sha1_final   (struct kc_sha1_t* context, uint8_t msg_digest[SHA1_HASH_SIZE]);

//---------------------------------------------------------------------------//

// SHA1 circular left shift macro
#define SHA1_CIRCULAR_SHIFT(bits, word)           \
  (((word) << (bits)) | ((word) >> (32-(bits))))

//---------------------------------------------------------------------------//

#endif /* KC_SHA1_T_H */
