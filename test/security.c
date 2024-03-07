// This file is part of keepcoding_core
// ==================================
//
// security.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../hdrs/security/base64.h"
#include "../hdrs/security/uuid.h"
#include "../hdrs/security/md5.h"
#include "../hdrs/common.h"
#include "../hdrs/test.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

// ------------------ B64 ------------------ //

int check_encode_case(char* input, char* output)
{
  char* out;
  int ret = kc_base64_encode(input, sizeof(char) * strlen(input), &out);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }
  ok(strcmp(out, output) == 0);
  free(out);

  return KC_SUCCESS;
}

int check_decode_case(char* input, char* output)
{
  char* out;
  int ret = kc_base64_decode(input, sizeof(char) * strlen(input), &out);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }
  ok(strcmp(out, output) == 0);
  free(out);

  return KC_SUCCESS;
}

// ------------------ MD5 ------------------ //

#define TEST_BLOCK_LEN   1000
#define TEST_BLOCK_COUNT 1000

void _md5_print(unsigned char digest[16])
{
  for (unsigned int i = 0; i < 16; ++i)
  {
    printf("%02x", digest[i]);
  }
}

int _check_hex_final(unsigned char digest[16], const char* expected)
{
  unsigned char out[16];
  unsigned int temp;

  for (int i = 0; i < 16; ++i)
  {
    if (sscanf(expected + 2 * i, "%02x", &temp) != 1)
    {
      return KC_FATAL_LOG_ERROR;
    }

    out[i] = (unsigned char)temp;
  }

  if (memcmp(digest, out, 16) != 0)
  {
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

int _check_md5_final(char* str, const char* expected)
{
  struct kc_md5_t context;
  unsigned char digest[16];
  unsigned int len = strlen(str);

  md5_init(&context);
  md5_update(&context, str, len);
  md5_final(digest, &context);

  return _check_hex_final(digest, expected);
}

int main()
{
  //testgroup("Base64")
  //{
  //  subtest("kc_base64_encode()")
  //  {
  //    ok(check_encode_case("", "") == KC_SUCCESS);
  //    ok(check_encode_case("f", "Zg==") == KC_SUCCESS);
  //    ok(check_encode_case("fo", "Zm8=") == KC_SUCCESS);
  //    ok(check_encode_case("foo", "Zm9v") == KC_SUCCESS);
  //    ok(check_encode_case("foob", "Zm9vYg==") == KC_SUCCESS);
  //    ok(check_encode_case("fooba", "Zm9vYmE=") == KC_SUCCESS);
  //    ok(check_encode_case("foobar", "Zm9vYmFy") == KC_SUCCESS);
  //  }

  //  subtest("kc_base64_decode()")
  //  {
  //    ok(check_decode_case("", "") == KC_SUCCESS);
  //    ok(check_decode_case("Zg==", "f") == KC_SUCCESS);
  //    ok(check_decode_case("Zm8=", "fo") == KC_SUCCESS);
  //    ok(check_decode_case("Zm9v", "foo") == KC_SUCCESS);
  //    ok(check_decode_case("Zm9vYg==", "foob") == KC_SUCCESS);
  //    ok(check_decode_case("Zm9vYmE=", "fooba") == KC_SUCCESS);
  //    ok(check_decode_case("Zm9vYmFy", "foobar") == KC_SUCCESS);
  //  }

  //  done_testing();
  //}

  //testgroup("UUID")
  //{
  //  done_testing();
  //}

  testgroup("MD5")
  {
    subtest("md5 string")
    {
      struct kc_md5_t context;
      unsigned char digest[16];
      unsigned int len = strlen("test");

      md5_init(&context);
      md5_update(&context, "test", len);
      md5_final(digest, &context);

      const unsigned char out[16] = 
      { 
        0x09, 0x8f, 0x6b, 0xcd, 0x46, 0x21, 0xd3, 0x73, 
        0xca, 0xde, 0x4e, 0x83, 0x26, 0x27, 0xb4, 0xf6 
      };

      ok(memcmp(digest, out, 16) == KC_SUCCESS);
    }

    subtest("md5 time trial")
    {
      struct kc_md5_t context;
      time_t endTime, startTime;
      unsigned char block[TEST_BLOCK_LEN];
      unsigned char digest[16];

      for (unsigned int i = 0; i < TEST_BLOCK_LEN; ++i)
      {
        block[i] = (unsigned char)(i & 0xff);
      }

      time(&startTime);
      md5_init(&context);

      for (unsigned int i = 0; i < TEST_BLOCK_COUNT; ++i)
      {
        md5_update(&context, block, TEST_BLOCK_LEN);
      }

      md5_final(digest, &context);

      time(&endTime);

      ok((long)(endTime - startTime) <= 0);
    }

    subtest("md5 test suite")
    {
      ok(_check_md5_final("", "d41d8cd98f00b204e9800998ecf8427e") == KC_SUCCESS);
      ok(_check_md5_final("a", "0cc175b9c0f1b6a831c399e269772661") == KC_SUCCESS);
      ok(_check_md5_final("abc", "900150983cd24fb0d6963f7d28e17f72") == KC_SUCCESS);
      ok(_check_md5_final("message digest", "f96b697d7cb7938d525a2f31aaf161d0") == KC_SUCCESS);
      ok(_check_md5_final("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b") == KC_SUCCESS);
      ok(_check_md5_final("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f") == KC_SUCCESS);
      ok(_check_md5_final("1234567890123456789012345678901234567890\1234567890123456789012345678901234567890", "ab56e40dae99a018623a018ea92693cd") == KC_SUCCESS);
    }

    subtest("md5 file")
    {
      skip(true)
      {
        FILE* file;
        unsigned char buffer[1024], digest[16];

        if ((file = fopen("test", "rb")) == NULL)
        {
          printf("%s can't be opened\n", "test");
        }
        else
        {
          struct kc_md5_t context;
          md5_init(&context);

          int len = 0;

          while (len = fread(buffer, 1, 1024, file))
          {
            md5_update(&context, buffer, len);
          }

          md5_final(digest, &context);

          fclose(file);
        }
      }
    }

    done_testing();
  }

  return 0;
}

