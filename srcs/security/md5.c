// This file is part of keepcoding_core
// ==================================
//
// uuid.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License
//
// RSA Data Security, Inc. MD5 Message-Digest Algorithm

#include "../../hdrs/security/md5.h"

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

void md5_init   PROTO_LIST((struct kc_md5_t* context));
void md5_update PROTO_LIST((struct kc_md5_t* context, unsigned char* input, unsigned int in_len));
void md5_final  PROTO_LIST((unsigned char[16] digest, struct kc_md5_t* context));

//--- MARK: PRIVATE FUNCTION PROTOTYPES --------------------------------------//

static void _md5_transform PROTO_LIST((UINT4[4], unsigned char[64]));
static void _encode        PROTO_LIST((unsigned char*, UINT4*, unsigned int));
static void _decode        PROTO_LIST((UINT4*, unsigned char*, unsigned int));
static void _md5_memcpy    PROTO_LIST((POINTER, POINTER, unsigned int));
static void _md5_memset    PROTO_LIST((POINTER, int, unsigned int));

//---------------------------------------------------------------------------//

static unsigned char PADDING[64] = 
{
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//---------------------------------------------------------------------------//

// MD5 initialization. Begins an MD5 operation, writing a new context
void md5_init(struct kc_md5_t* context)
{
  context->count[0] = context->count[1] = 0;

  // Load magic initialization constants.
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

//---------------------------------------------------------------------------//

// MD5 block update operation. Continues an MD5 message-digest
// operation, processing another message block, and updating the context
void md5_update(struct kc_md5_t* context, unsigned char* input, unsigned int in_len)
{
  unsigned int i, index, part_len;

  // compute number of bytes mod 64
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  // update number of bits
  if ((context->count[0] += ((UINT4)in_len << 3)) < ((UINT4)in_len << 3))
  {
    context->count[1]++;
  }

  context->count[1] += ((UINT4)in_len >> 29);

  part_len = 64 - index;

  // transform as many times as possible.
  if (in_len >= part_len)
  {
    _md5_memcpy((POINTER)&context->buffer[index], (POINTER)input, part_len);
    _md5_transform(context->state, context->buffer);

    for (i = part_len; i + 63 < in_len; i += 64)
    {
      _md5_transform(context->state, &input[i]);
    }

    index = 0;
  }
  else
  {
    i = 0;
  }

  // buffer remaining input
  _md5_memcpy((POINTER)&context->buffer[index], (POINTER)&input[i], in_len - i);
}

//---------------------------------------------------------------------------//

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context
void md5_final(unsigned char digest[16], struct kc_md5_t* context)
{
  unsigned char bits[8];
  unsigned int index, padLen;

  // save number of bits
  _encode(bits, context->count, 8);

  // pad out to 56 mod 64
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  md5_update(context, PADDING, padLen);

  // append length (before padding)
  md5_update(context, bits, 8);

  // store state in digest
  _encode(digest, context->state, 16);

  // zeroize sensitive information.
  _md5_memset((POINTER)context, 0, sizeof(*context));
}

//---------------------------------------------------------------------------//

// MD5 basic transformation. Transforms state based on block
static void _md5_transform(UINT4 state[4], unsigned char block[64])
{
  UINT4 a = state[0];
  UINT4 b = state[1];
  UINT4 c = state[2];
  UINT4 d = state[3];

  UINT4 x[16];

  _decode(x, block, 64);

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

  // Zeroize sensitive information.
  _md5_memset((POINTER)x, 0, sizeof(x));
}

//---------------------------------------------------------------------------//

// Encodes input (UINT4) into output (unsigned char). Assumes len is a multiple of 4
static void _encode(unsigned char* output, UINT4* input, unsigned int len)
{
  for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
  {
    output[j] = (unsigned char)(input[i] & 0xff);
    output[j + 1] = (unsigned char)((input[i] >> 8) & 0xff);
    output[j + 2] = (unsigned char)((input[i] >> 16) & 0xff);
    output[j + 3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

//---------------------------------------------------------------------------//

// Decodes input (unsigned char) into output (UINT4). 
// Assumes len is a multiple of 4
static void _decode(UINT4* output, unsigned char* input, unsigned int len)
{
  for (unsigned int i = 0, j = 0; j < len; ++i, j += 4)
  {
    output[i] = ((UINT4)input[j]) | (((UINT4)input[j + 1]) << 8) | 
      (((UINT4)input[j + 2]) << 16) | (((UINT4)input[j + 3]) << 24);
  }
}

//---------------------------------------------------------------------------//

static void _md5_memcpy(POINTER output, POINTER input, unsigned int len)
{
#ifdef __STDC_NO_MEMORY__
  // If __STDC_NO_MEMORY__ is defined, 'memcpy' is not available
  for (unsigned int i = 0; i < len; ++i)
  {
    output[i] = input[i];
  }
#else
  memcpy(output, input, len);
#endif
}

//---------------------------------------------------------------------------//

static void _md5_memset(POINTER output, int value, unsigned int len)
{
#ifdef __STDC_NO_MEMORY__
  // If __STDC_NO_MEMORY__ is defined, 'memset' is not available
  for (unsigned int i = 0; i < len; ++i)
  {
    ((char*)output)[i] = (char)value;
  }
#else
  memset(output, value, len);
#endif
}

//---------------------------------------------------------------------------//