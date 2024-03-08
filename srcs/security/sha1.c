// This file is part of keepcoding_core
// ==================================
//
// sha1.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License


#include "../../hdrs/security/sha1.h"

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int sha1_init  (struct kc_sha1_t* context);
int sha1_update  (struct kc_sha1_t* context, const uint8_t* msg_arrey, unsigned int len);
int sha1_final (struct kc_sha1_t* context, uint8_t msg_digest[SHA1_HASH_SIZE]);

//--- MARK: PRIVATE FUNCTION PROTOTYPES --------------------------------------//

void _sha1_process_message_block(struct kc_sha1_t* context);
void _sha1_pad_message(struct kc_sha1_t* context);

//---------------------------------------------------------------------------//

int sha1_init(struct kc_sha1_t* context)
{
  if (context == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  context->length_low = 0;
  context->length_high = 0;
  context->message_block_index = 0;

  context->intermediate_hash[0] = 0x67452301;
  context->intermediate_hash[1] = 0xEFCDAB89;
  context->intermediate_hash[2] = 0x98BADCFE;
  context->intermediate_hash[3] = 0x10325476;
  context->intermediate_hash[4] = 0xC3D2E1F0;

  context->computed = 0;
  context->corrupted = 0;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int sha1_update(struct kc_sha1_t* context, const uint8_t* msg_array, unsigned int len)
{
  if (len == 0)
  {
    return KC_SUCCESS;
  }

  if (context == NULL || msg_array == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  if (context->computed)
  {
    context->corrupted = KC_SHA1_STATE_ERROR;
    return KC_SHA1_STATE_ERROR;
  }

  if (context->corrupted)
  {
    return context->corrupted;
  }

  while (len-- && !context->corrupted)
  {
    context->message_block[context->message_block_index++] = (*msg_array & 0xFF);
    context->length_low += 8;

    if (context->length_low == 0)
    {
      context->length_high++;
      if (context->length_high == 0)
      {
        // Message is too long
        context->corrupted = 1;
      }
    }

    if (context->message_block_index == 64)
    {
      _sha1_process_message_block(context);
    }

    msg_array++;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int sha1_final(struct kc_sha1_t* context, uint8_t msg_digest[SHA1_HASH_SIZE])
{
  int i;

  if (context == NULL || msg_digest == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  if (context->corrupted)
  {
    return context->corrupted;
  }

  if (!context->computed)
  {
    _sha1_pad_message(context);

    // message may be sensitive, clear it out
    for (i = 0; i < 64; ++i)
    {
      context->message_block[i] = 0;
    }

    // and clear length
    context->length_low = 0;
    context->length_high = 0;
    context->computed = 1;
  }

  for (i = 0; i < SHA1_HASH_SIZE; ++i)
  {
    msg_digest[i] = context->intermediate_hash[i >> 2] >> 8 * (3 - (i & 0x03));
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

void _sha1_process_message_block(struct kc_sha1_t* context)
{
  // Constants defined in SHA-1
  const uint32_t K[] = 
  { 
    0x5A827999,
    0x6ED9EBA1,
    0x8F1BBCDC,
    0xCA62C1D6
  };

  int      t;                 // Loop counter                
  uint32_t temp;              // Temporary word value        
  uint32_t W[80];             // Word sequence               
  uint32_t A, B, C, D, E;     // Word buffers      

  // initialize the first 16 words in the array W
  for (t = 0; t < 16; ++t)
  {
    W[t] = context->message_block[t * 4] << 24;
    W[t] |= context->message_block[t * 4 + 1] << 16;
    W[t] |= context->message_block[t * 4 + 2] << 8;
    W[t] |= context->message_block[t * 4 + 3];
  }

  for (t = 16; t < 80; ++t)
  {
    W[t] = SHA1_CIRCULAR_SHIFT(1, W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16]);
  }

  A = context->intermediate_hash[0];
  B = context->intermediate_hash[1];
  C = context->intermediate_hash[2];
  D = context->intermediate_hash[3];
  E = context->intermediate_hash[4];

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

  context->intermediate_hash[0] += A;
  context->intermediate_hash[1] += B;
  context->intermediate_hash[2] += C;
  context->intermediate_hash[3] += D;
  context->intermediate_hash[4] += E;

  context->message_block_index = 0;
}

//---------------------------------------------------------------------------//

void _sha1_pad_message(struct kc_sha1_t* context)
{
  // check to see if the current message block is too small to hold
  // the initial padding bits and length
  // 
  // if so, we will pad the block, process it, and then continue 
  // padding into a second block
  
  if (context->message_block_index > 55)
  {
    context->message_block[context->message_block_index++] = 0x80;

    while (context->message_block_index < 64)
    {
      context->message_block[context->message_block_index++] = 0;
    }

    _sha1_process_message_block(context);

    while (context->message_block_index < 56)
    {
      context->message_block[context->message_block_index++] = 0;
    }
  }
  else
  {
    context->message_block[context->message_block_index++] = 0x80;

    while (context->message_block_index < 56)
    {
      context->message_block[context->message_block_index++] = 0;
    }
  }

  // store the message length as the last 8 octets
  context->message_block[56] = context->length_high >> 24;
  context->message_block[57] = context->length_high >> 16;
  context->message_block[58] = context->length_high >> 8;
  context->message_block[59] = context->length_high;

  context->message_block[60] = context->length_low >> 24;
  context->message_block[61] = context->length_low >> 16;
  context->message_block[62] = context->length_low >> 8;
  context->message_block[63] = context->length_low;

  _sha1_process_message_block(context);
}

//---------------------------------------------------------------------------//
