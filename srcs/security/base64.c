// This file is part of keepcoding_core
// ==================================
//
// base64.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/security/base64.h"

#include <stdlib.h>
#include <stdbool.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

int kc_base64_encode  (const unsigned char* src, size_t src_len, char** output);
int kc_base64_decode  (const unsigned char* src, size_t src_len, char** output);

//---------------------------------------------------------------------------//

static const unsigned char _base64_table[65] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static const unsigned int _base64_index[256] = 
{ 
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  62, 63, 62, 62, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
  61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,
  63, 0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
  43, 44, 45, 46, 47, 48, 49, 50, 51
};

//---------------------------------------------------------------------------//

int kc_base64_encode(const unsigned char* src, size_t src_len, char** output)
{
  const unsigned char* end     = src + src_len;
  const unsigned char* tmp_in  = src;

  // 3-byte blocks to 4-byte
  unsigned int _3b2_4b = (src_len * 4 / 3 + 4);
  size_t out_len = _3b2_4b + (_3b2_4b / 72) + 1;

  if (out_len < src_len)
  {
    return KC_OVERFLOW;
  }

  (*output) = malloc(sizeof(char) * out_len);

  if (output == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  unsigned char* pos = (*output);

  while (end - tmp_in >= 3)
  {
    *pos++ = _base64_table[tmp_in[0] >> 2];
    *pos++ = _base64_table[((tmp_in[0] & 0x03) << 4) | (tmp_in[1] >> 4)];
    *pos++ = _base64_table[((tmp_in[1] & 0x0f) << 2) | (tmp_in[2] >> 6)];
    *pos++ = _base64_table[tmp_in[2] & 0x3f];

    tmp_in += 3;
  }

  if (end - tmp_in)
  {
    *pos++ = _base64_table[tmp_in[0] >> 2];

    if (end - tmp_in == 1)
    {
      *pos++ = _base64_table[(tmp_in[0] & 0x03) << 4];
      *pos++ = '=';
    }
    else 
    {
      *pos++ = _base64_table[((tmp_in[0] & 0x03) << 4) | (tmp_in[1] >> 4)];
      *pos++ = _base64_table[(tmp_in[1] & 0x0f) << 2];
    }

    *pos++ = '=';
  }

  // Null-terminate the string
  *pos = '\0';

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int kc_base64_decode(const unsigned char* src, size_t src_len, char** output)
{   
  unsigned char* tmp_in = src;

  int padding = src_len > 0 && (src_len % 4 || tmp_in[src_len - 1] == '=');

  // 4-byte blocks to 3-byte
  unsigned int _4b2_3b = (src_len + 3) / 4;
  (*output) = malloc(sizeof(char) * (_4b2_3b * 3 - padding) + 1);
  
  if (*output == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  size_t L = ((src_len + 3) / 4 - padding) * 4;
  unsigned char* pos = (*output);

  for (size_t i = 0; i < L; i += 4)
  {
    int n = (_base64_index[tmp_in[i]] << 18) 
      | (_base64_index[tmp_in[i + 1]] << 12) 
      | (_base64_index[tmp_in[i + 2]] << 6) 
      | (_base64_index[tmp_in[i + 3]]);

    *pos++ = (n >> 16);
    *pos++ = (n >> 8 & 0xFF);
    *pos++ = (n & 0xFF);
  }

  if (padding != 0)
  {
    int n = (_base64_index[tmp_in[L]] << 18) 
      | (_base64_index[tmp_in[L + 1]] << 12);

    *pos++ = (n >> 16);

    if (src_len > L + 2 && tmp_in[L + 2] != '=')
    {
      n |= _base64_index[tmp_in[L + 2]] << 6;
      *pos++ = (n >> 8 & 0xFF);
    }
  }

  // Null-terminate the string
  *pos = '\0';

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

