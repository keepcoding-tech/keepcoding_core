// This file is part of keepcoding_core
// ==================================
//
// security.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#define _CRT_SECURE_NO_WARNINGS

#include "../hdrs/security/base64.h"
#include "../hdrs/security/md5.h"
#include "../hdrs/security/sha1.h"
#include "../hdrs/security/uuid.h"
#include "../hdrs/common.h"
#include "../hdrs/test.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

// ------------------ global ----------------- //

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

int _check_md5_final(char* str, const char* expected)
{
  struct kc_md5_t context;
  unsigned char digest[16];
  unsigned int len = strlen(str);

  md5_init(&context);
  md5_update(&context, str, len);
  md5_final(&context, digest);

  return _check_hex_final(digest, expected);
}

// ------------------ SHA1 ----------------- //

#define TEST1   "abc"

#define TEST2a  "abcdbcdecdefdefgefghfghighijhi"
#define TEST2b  "jkijkljklmklmnlmnomnopnopq"
#define TEST2   TEST2a TEST2b

#define TEST3   "a"

#define TEST4a  "01234567012345670123456701234567"
#define TEST4b  "01234567012345670123456701234567"
#define TEST4   TEST4a TEST4b

// ------------------ UUID ----------------- //

void _print_uuid(struct kc_uuid_t u)
{
  printf("%8.8x-", u.time_low);
  printf("%4.4x-", u.time_mid);
  printf("%4.4x-", u.time_hi_and_version);
  printf("%2.2x",  u.clock_seq_hi_and_reserved);
  printf("%2.2x-", u.clock_seq_low);

  for (int i = 0; i < 6; ++i)
  {
    printf("%02x", (unsigned char)u.node[i]);
  }

  printf("\n");
}

int _compare_uuids(struct kc_uuid_t u1, struct kc_uuid_t u2)
{
  int comp = uuid_compare(&u1, &u1);
  if (comp != 0)
  {
    return KC_INVALID;
  }

  comp = uuid_compare(&u2, &u2);
  if (comp != 0)
  {
    return KC_INVALID;
  }

  comp = uuid_compare(&u1, &u2);
  if (comp == 0)
  {
    return KC_INVALID;
  }

  comp = uuid_compare(&u2, &u1);
  if (comp == 0)
  {
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

int _check_uuid(struct kc_uuid_t namespace_uuid)
{
  struct kc_uuid_t u1, u2;

  for (int i = 0; i < TEST_BLOCK_LEN; ++i)
  {
    uuid_create(&u1);
    uuid_create(&u2);

    int ret = _compare_uuids(u1, u2);
    if (ret != KC_SUCCESS)
    {
      return KC_INVALID;
    }
  }

  for (int i = 0; i < TEST_BLOCK_LEN; ++i)
  {
    uuid_create_md5_from_name(&u1, namespace_uuid, "www.widgets.com", 15);
    uuid_create_md5_from_name(&u2, namespace_uuid, "www.widgets.com", 15);

    int ret = _compare_uuids(u1, u2);
    if (ret != KC_SUCCESS)
    {
      return KC_INVALID;
    }
  }

  return KC_SUCCESS;
}

int main()
{
  testgroup("B64")
  {
    subtest("kc_base64_encode()")
    {
      ok(check_encode_case("", "") == KC_SUCCESS);
      ok(check_encode_case("f", "Zg==") == KC_SUCCESS);
      ok(check_encode_case("fo", "Zm8=") == KC_SUCCESS);
      ok(check_encode_case("foo", "Zm9v") == KC_SUCCESS);
      ok(check_encode_case("foob", "Zm9vYg==") == KC_SUCCESS);
      ok(check_encode_case("fooba", "Zm9vYmE=") == KC_SUCCESS);
      ok(check_encode_case("foobar", "Zm9vYmFy") == KC_SUCCESS);
    }

    subtest("kc_base64_decode()")
    {
      ok(check_decode_case("", "") == KC_SUCCESS);
      ok(check_decode_case("Zg==", "f") == KC_SUCCESS);
      ok(check_decode_case("Zm8=", "fo") == KC_SUCCESS);
      ok(check_decode_case("Zm9v", "foo") == KC_SUCCESS);
      ok(check_decode_case("Zm9vYg==", "foob") == KC_SUCCESS);
      ok(check_decode_case("Zm9vYmE=", "fooba") == KC_SUCCESS);
      ok(check_decode_case("Zm9vYmFy", "foobar") == KC_SUCCESS);
    }

    done_testing();
  }

  testgroup("MD5")
  {
    subtest("md5 string")
    {
      struct kc_md5_t context;
      unsigned char digest[16];
      unsigned int len = strlen("test");

      md5_init(&context);
      md5_update(&context, "test", len);
      md5_final(&context, digest);

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

      md5_final(&context, digest);

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
      FILE* file;

      if ((file = fopen("test", "rb")) == NULL)
      {
        printf("%s can't be opened\n", "test");
      }
      else
      {
        struct kc_md5_t context;
        md5_init(&context);

        unsigned char buffer[1024];
        unsigned char digest[16];
        int len = 0;

        while (len = fread(buffer, 1, 1024, file))
        {
          md5_update(&context, buffer, len);
        }

        md5_final(&context, digest);

        fclose(file);
      }
    }

    done_testing();
  }

  testgroup("SHA1")
  {
    subtest("sha1 test suite")
    {
      long int repeat_count[4] = { 1, 1, 1000000, 10 };
      char* test_arr[4] = { TEST1, TEST2, TEST3, TEST4 };
      char* result_arr[4] =
      {
        "A9993E364706816ABA3E25717850C26C9CD0D89D",
        "84983E441C3BD26EBAAE4AA1F95129E5E54670F1",
        "34AA973CD4C4DAA4F61EEB2BDBAD27316534016F",
        "DEA356A2CDDD90C7A7ECEDC5EBB563934F460452"
      };

      struct kc_sha1_t sha;
      uint8_t digest[20];

      int ret = KC_INVALID;

      for (int j = 0; j < 4; ++j)
      {
        ret = sha1_init(&sha);
        ok(ret == KC_SUCCESS);

        for (int i = 0; i < repeat_count[j]; ++i)
        {
          ret = sha1_update(&sha, 
            (const unsigned char*)test_arr[j], 
            strlen(test_arr[j]));

          ok(ret == KC_SUCCESS);
        }

        ret = sha1_final(&sha, digest);
        ok(ret == KC_SUCCESS);

        ok(_check_hex_final(digest, result_arr[j]) == KC_SUCCESS);
      }

      /* Test some error returns */
      ret = sha1_update(&sha, (const unsigned char*)test_arr[1], 1);
      ok(ret == KC_SHA1_STATE_ERROR);

      ret = sha1_init(0);
      ok(ret == KC_NULL_REFERENCE);
    }

    done_testing();
  }

  testgroup("UUID")
  {
    subtest("namespace DNS")
    {
      // Name string is a fully-qualified domain name
      // 6ba7b810-9dad-11d1-80b4-00c04fd430c8
      struct kc_uuid_t namespace_DNS = 
      {
          0x6ba7b810,
          0x9dad,
          0x11d1,
          0x80, 
          0xb4, 
          0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8
      };

      ok(_check_uuid(namespace_DNS));
    }

    subtest("namespace URL")
    {
      // Name string is a URL
      // 6ba7b811-9dad-11d1-80b4-00c04fd430c8
      struct kc_uuid_t namespace_URL = 
      {
          0x6ba7b811,
          0x9dad,
          0x11d1,
          0x80, 
          0xb4, 
          0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8
      };

      ok(_check_uuid(namespace_URL));
    }

    subtest("namespace OID")
    {
      // Name string is an ISO OID
      // 6ba7b812-9dad-11d1-80b4-00c04fd430c8
      struct kc_uuid_t namespace_OID = 
      {
          0x6ba7b812,
          0x9dad,
          0x11d1,
          0x80, 
          0xb4, 
          0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8
      };

      ok(_check_uuid(namespace_OID));
    }

    subtest("namespace X500")
    {
      // Name string is an X.500 DN (in DER or a text output format)
      // 6ba7b814-9dad-11d1-80b4-00c04fd430c8
      struct kc_uuid_t namespace_X500 = 
      {
          0x6ba7b814,
          0x9dad,
          0x11d1,
          0x80, 
          0xb4, 
          0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8
      };

      ok(_check_uuid(namespace_X500));
    }

    done_testing();
  }

  return 0;
}

