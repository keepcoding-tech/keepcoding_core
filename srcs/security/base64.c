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

int kc_base64_encode  (const BYTE* buffer, unsigned int buffer_len, char** output);
int kc_base64_decode  (const char* input, BYTE** output);

int kc_base64_index   (char c);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static bool _is_base64_char    (BYTE b);
static BYTE _find_base64_char  (BYTE b);

const char* _base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz"
                            "0123456789+/";

//---------------------------------------------------------------------------//

int kc_base64_encode(const BYTE* buffer, unsigned int buffer_len, char** output)
{
  int i = 0;
  int j = 0;
  int o = 0;

  BYTE char_array_3[3];
  BYTE char_array_4[4];

  while (buffer_len--)
  {
    char_array_3[i++] = *(buffer++);

    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; i < 4; ++i)
      {
        (*output)[o++] = _base64_chars[char_array_4[i]];
      }

      i = 0;
    }
  }

  if (i)
  {
    for (j = i; j < 3; ++j)
    {
      char_array_3[j] = '\0';
    }

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; j < i + 1; ++j)
    {
      (*output)[o++] += _base64_chars[char_array_4[j]];
    }

    while ((i++ < 3))
    {
      (*output)[o++] = '=';
    }
  }

  return 0;
}

//---------------------------------------------------------------------------//

int kc_base64_decode(const char* input, BYTE** output)
{   
  int i = 0;
  int j = 0;
  int o = 0;
  int in_ = 0;

  int in_len = strlen(input);

  BYTE char_array_4[4], char_array_3[3];

  while (in_len-- && (input[in_] != '=') && _is_base64_char(input[in_]))
  {
    char_array_4[i++] = input[in_];
    in_++;

    if (i == 4)
    {
      for (i = 0; i < 4; ++i)
      {
        char_array_4[i] = _find_base64_char(char_array_4[i]);
      }

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; i < 3; ++i)
      {
        (*output)[o++] = char_array_3[i];
      }

      i = 0;
    }
  }

  if (i)
  {
    for (j = i; j < 4; ++j)
    {
      char_array_4[j] = 0;
    }

    for (j = 0; j < 4; j++)
    {
      char_array_4[j] = _find_base64_char(char_array_4[j]);
    }

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; j < i - 1; ++j)
    {
      (*output)[o++] = char_array_3[j];
    }
  }

  return 0;
}

//---------------------------------------------------------------------------//

int kc_base64_index(char c)
{
  const char* p = strchr(_base64_chars, c);
  return (p != NULL) ? (int)(p - _base64_chars) : -1;
}

//---------------------------------------------------------------------------//

bool _is_base64_char(BYTE b)
{
   return (isalnum(b) || (b == '+') || (b == '/'));
}

//---------------------------------------------------------------------------//

BYTE _find_base64_char(BYTE b)
{
  for (int i = 0; i < KC_BASE64_LEN; ++i)
  {
    if (_base64_chars[i] == b)
    {
      return _base64_chars[i];
    }
  }

  return -1;
}

//---------------------------------------------------------------------------//
