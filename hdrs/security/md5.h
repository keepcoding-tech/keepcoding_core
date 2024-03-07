// This file is part of keepcoding_core
// ==================================
//
// md5.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License
//
// RSA Data Security, Inc. MD5 Message-Digest Algorithm

/*
 * The MD5 algorithm is designed to be quite fast on 32-bit machines. In
 * addition, the MD5 algorithm does not require any large substitution
 * tables; the algorithm can be coded quite compactly.
 * 
 * The MD5 algorithm is an extension of the MD4 message-digest algorithm.
 * MD5 is slightly slower than MD4, but is more "conservative" in
 * design. MD5 was designed because it was felt that MD4 was perhaps
 * being adopted for use more quickly than justified by the existing
 * critical review; because MD4 was designed to be exceptionally fast,
 * it is "at the edge" in terms of risking successful cryptanalytic
 * attack.
 * 
 * MD5 backs off a bit, giving up a little in speed for a much
 * greater likelihood of ultimate security. It incorporates some
 * suggestions made by various reviewers, and contains additional
 * optimizations. The MD5 algorithm is being placed in the public domain
 * for review and possible adoption as a standard.
 */

#ifndef KC_MD5_T_H
#define KC_MD5_T_H

#include "../common.h"

//---------------------------------------------------------------------------//

struct kc_md5_t
{
  unsigned long int state[4];    // state (ABCD)
  unsigned long int count[2];    // number of bits, modulo 2^64 (lsb first)
  unsigned char     buffer[64];  // input buffer
};

//---------------------------------------------------------------------------//

void md5_init   PROTO_LIST((struct kc_md5_t* context));
void md5_update PROTO_LIST((struct kc_md5_t* context, unsigned char* input, unsigned int in_len));
void md5_final  PROTO_LIST((unsigned char digest[16], struct kc_md5_t* context));

//---------------------------------------------------------------------------//

// constants for md5_transform
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

// F, G, H and I are basic MD5 functions
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ROTATE_LEFT rotates x left n bits
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4. 
// Rotation is separate from addition to prevent recomputation.
#define FF(a, b, c, d, x, s, ac)                \
{                                               \
  (a) += F ((b), (c), (d)) + (x) + (UINT4)(ac); \
  (a) = ROTATE_LEFT ((a), (s));                 \
  (a) += (b);                                   \
}

#define GG(a, b, c, d, x, s, ac)                \
{                                               \
  (a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
  (a) = ROTATE_LEFT ((a), (s));                 \
  (a) += (b);                                   \
}
#define HH(a, b, c, d, x, s, ac)                \
{                                               \
  (a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
  (a) = ROTATE_LEFT ((a), (s));                 \
  (a) += (b);                                   \
}

#define II(a, b, c, d, x, s, ac)                \
{                                               \
  (a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
  (a) = ROTATE_LEFT ((a), (s));                 \
  (a) += (b);                                   \
}

//---------------------------------------------------------------------------//

#endif /* KC_MD5_T_H */
