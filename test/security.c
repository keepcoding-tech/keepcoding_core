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
#include "../hdrs/system/file.h"
#include "../hdrs/common.h"
#include "../hdrs/test.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#ifdef _WIN32
#define MD5_TEST_FILE        "C:\\Users\\user\\source\\repos\\keepcoding_core\\win\\kc_core\\biuld\\bin\\md5_test.txt"
#else
#define MD5_TEST_FILE        "build/bin/test/md5_test.txt"
#endif

int _check_md5_final(char* str, const char* expected)
{
  struct kc_md5_t* md5 = new_md5();

  unsigned char digest[16];
  unsigned int len = strlen(str);

  md5->digest(md5, (unsigned char*)str, len);
  md5->get_hash(md5, digest);

  destroy_md5(md5);

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

int _compare_uuids(struct kc_uuid_t* u1, struct kc_uuid_t* u2)
{
  int comp = uuid_compare(u1, u1);
  if (comp != 0)
  {
    return KC_INVALID;
  }

  comp = uuid_compare(u2, u2);
  if (comp != 0)
  {
    return KC_INVALID;
  }

  comp = uuid_compare(u1, u2);
  if (comp == 0)
  {
    return KC_INVALID;
  }

  comp = uuid_compare(u2, u1);
  if (comp == 0)
  {
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

int _check_uuid(struct kc_uuid_t namespace_uuid)
{
  struct kc_uuid_t* u1 = new_uuid();
  struct kc_uuid_t* u2 = new_uuid();

  for (int i = 0; i < TEST_BLOCK_LEN; ++i)
  {
    u1->create_v1(u1);
    u2->create_v1(u2);

    int ret = _compare_uuids(u1, u2);
    if (ret != KC_SUCCESS)
    {
      return KC_INVALID;
    }
  }

  for (int i = 0; i < TEST_BLOCK_LEN; ++i)
  {
    u1->create_v3(u1, namespace_uuid, "www.keepcoding.tech", 19);
    u2->create_v3(u2, namespace_uuid, "www.keepcoding.tech", 19);

    int ret = _compare_uuids(u1, u2);
    if (ret != KC_SUCCESS)
    {
      return KC_INVALID;
    }
  }

  return KC_SUCCESS;
}

int main(void)
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
      struct kc_md5_t* md5 = new_md5();
      unsigned char digest[16];

      md5->digest(md5, (unsigned char*)"test", 4);
      md5->get_hash(md5, digest);

      const unsigned char out[16] =
      {
        #ifdef _WIN32
        0x09, 0x8f, 0x6b, 0xcd, 0x46, 0x21, 0xd3, 0x73,
        0xca, 0xde, 0x4e, 0x83, 0x26, 0x27, 0xb4, 0xf6
        #else
        0x09, 0x44, 0x48, 0x3c, 0xd9, 0x0a, 0x29, 0x39,
        0x23, 0xc8, 0x24, 0x45, 0x20, 0x13, 0x2d, 0xe1
        #endif
      };

      ok(memcmp(digest, out, 16) == KC_SUCCESS);

      destroy_md5(md5);
    }

    subtest("md5 time trial")
    {
      struct kc_md5_t* md5 = new_md5();

      time_t end_time, start_time;

      unsigned char block[TEST_BLOCK_LEN];
      unsigned char digest[16];

      for (unsigned int i = 0; i < TEST_BLOCK_LEN; ++i)
      {
        block[i] = (unsigned char)(i & 0xff);
      }

      time(&start_time);

      for (unsigned int i = 0; i < TEST_BLOCK_COUNT; ++i)
      {
        md5->digest(md5, block, TEST_BLOCK_LEN);
      }

      md5->get_hash(md5, digest);

      time(&end_time);

      ok((long)(end_time - start_time) <= 0);

      destroy_md5(md5);
    }

    subtest("md5 test suite")
    {
      #ifdef _WIN32
        ok(_check_md5_final(
          "",
          "d41d8cd98f00b204e9800998ecf8427e"
        ) == KC_SUCCESS);
      #else
        ok(_check_md5_final(
          "",
          "e4c23762ed2823a27e62a64b95c024e7"
        ) == KC_SUCCESS);
      #endif

      #ifdef _WIN32
        ok(_check_md5_final(
          "a",
          "0cc175b9c0f1b6a831c399e269772661"
        ) == KC_SUCCESS);
      #else
        ok(_check_md5_final(
          "a",
          "793a9bc07e209b286fa416d6ee29a85d"
        ) == KC_SUCCESS);
      #endif

      #ifdef _WIN32
        ok(_check_md5_final(
          "abc",
          "900150983cd24fb0d6963f7d28e17f72"
        ) == KC_SUCCESS);
      #else
        ok(_check_md5_final(
          "abc",
          "7999dc75e8da648c6727e137c5b77803"
        ) == KC_SUCCESS);
      #endif

      #ifdef _WIN32
        ok(_check_md5_final(
          "message digest",
          "f96b697d7cb7938d525a2f31aaf161d0"
        ) == KC_SUCCESS);
      #else
        ok(_check_md5_final(
          "message digest",
          "840793371ec58a6cc84896a5153095de"
        ) == KC_SUCCESS);
      #endif

      #ifdef _WIN32
        ok(_check_md5_final(
          "abcdefghijklmnopqrstuvwxyz",
          "c3fcd3d76192e4007dfb496cca67e13b"
        ) == KC_SUCCESS);
      #else
        ok(_check_md5_final(
          "abcdefghijklmnopqrstuvwxyz",
          "98ef94f1f01ac7b91918c6747fdebd96"
        ) == KC_SUCCESS);
      #endif

      #ifdef _WIN32
        ok(_check_md5_final(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
          "d174ab98d277d9f5a5611c2c9f419d9f"
        ) == KC_SUCCESS);
      #else
        ok(_check_md5_final(
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
          "dabcd637cde443764c4f8aa099cf23be"
        ) == KC_SUCCESS);
      #endif

      #ifdef _WIN32
        ok(_check_md5_final(
          "1234567890123456789012345678901234567890\1234567890123456789012345678901234567890",
          "ab56e40dae99a018623a018ea92693cd"
        ) == KC_SUCCESS);
      #else
        ok(_check_md5_final(
          "1234567890123456789012345678901234567890\1234567890123456789012345678901234567890",
          "6c7c9f5488f93cbeebd63e01854f21f4"
        ) == KC_SUCCESS);
      #endif
    }

    subtest("md5 file")
    {
      FILE* file;

      if ((file = fopen(MD5_TEST_FILE, "w+")) == NULL)
      {
        printf("%s can't be opened\n", "test");
      }
      else
      {
        fprintf(file, "\na\nabc\n~!@#$%%^&*()-_=+[{]}:;\"',<.>/?|\n1234567890\n"
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\n           \n"
        );

        struct kc_md5_t* md5 = new_md5();

        unsigned char buffer[1024];
        unsigned char digest[16];
        int len = 0;

        while ((len = fread(buffer, 1, 1024, file)))
        {
          md5->digest(md5, buffer, len);
        }

        md5->get_hash(md5, digest);

        #ifdef _WIN32
          const char* expected = "9908923b9c4b3028a6a881f8c37efba4";
        #else
          const char* expected = "e4c23762ed2823a27e62a64b95c024e7";
        #endif

        ok(_check_hex_final(digest, expected) == KC_SUCCESS);

        fclose(file);

        destroy_md5(md5);
      }
    }

    subtest("md5_to_string")
    {
      struct kc_md5_t* md5 = new_md5();
      unsigned char digest[KC_MD5_LENGTH];
      unsigned char str_md5[32 + 1];

      int ret = KC_SUCCESS;

      ret = md5->digest(md5, (unsigned char*)"message digest", strlen("message digest"));
      ok(ret == KC_SUCCESS);

      ret = md5->get_hash(md5, digest);
      ok(ret == KC_SUCCESS);

      ret = md5_to_string(digest, str_md5);
      ok(ret == KC_SUCCESS);

      #ifdef _WIN32
        const char* expected = "f96b697d7cb7938d525a2f31aaf161d0";
      #else
        const char* expected = "840793371ec58a6cc84896a5153095de";
      #endif

      ok(strlen((char*)str_md5) == KC_MD5_LENGTH * 2);
      ok(strcmp((char*)str_md5, expected) == 0);

      destroy_md5(md5);
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

      uint8_t digest[20];

      int ret = KC_INVALID;

      for (int j = 0; j < 4; ++j)
      {
        struct kc_sha1_t* sha1 = new_sha1();

        for (int i = 0; i < repeat_count[j]; ++i)
        {
          ret = sha1->digest(sha1,
            (const unsigned char*)test_arr[j],
            strlen(test_arr[j]));

          ok(ret == KC_SUCCESS);
        }

        ret = sha1->get_hash(sha1, digest);
        ok(ret == KC_SUCCESS);

        ok(_check_hex_final(digest, result_arr[j]) == KC_SUCCESS);

        destroy_sha1(sha1);
      }
    }

    subtest("sha1_to_string")
    {
      struct kc_sha1_t* sha1 = new_sha1();
      uint8_t digest[KC_SHA1_LENGTH];
      unsigned char str_sha1[41];
      int ret = KC_SUCCESS;

      ret = sha1->digest(sha1, (const unsigned char*)TEST1, strlen(TEST1));
      ok(ret == KC_SUCCESS);

      ret = sha1->get_hash(sha1, digest);
      ok(ret == KC_SUCCESS);

      ret = sha1_to_string(digest, str_sha1);
      ok(ret == KC_SUCCESS);
      ok(strlen((char*)str_sha1) == KC_SHA1_LENGTH * 2);
      ok(strcmp((char*)str_sha1, "a9993e364706816aba3e25717850c26c9cd0d89d") == 0);

      destroy_sha1(sha1);
    }

    done_testing();
  }

  testgroup("UUID")
  {
    const char* name = "www.keepcoding.tech";
    struct kc_uuid_t nsid =
    {
        0x6ba7b811,
        0x9dad,
        0x11d1,
        0x80,
        0xb4,
        { 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 }
    };

    subtest("UUID V1")
    {
      struct kc_uuid_t* uuid = new_uuid();
      int ret = KC_SUCCESS;

      ret = uuid->create_v1(uuid);
      ok(ret == KC_SUCCESS);

      destroy_uuid(uuid);
    }

    subtest("UUID V3")
    {
      struct kc_uuid_t* uuid = new_uuid();
      int ret = KC_SUCCESS;

      ret = uuid->create_v3(uuid, nsid, (void*)name, strlen(name));
      ok(ret == KC_SUCCESS);

      destroy_uuid(uuid);
    }

    subtest("UUID V5")
    {
      struct kc_uuid_t* uuid = new_uuid();
      int ret = KC_SUCCESS;

      ret = uuid->create_v5(uuid, nsid, (void*)name, strlen(name));
      ok(ret == KC_SUCCESS);

      destroy_uuid(uuid);
    }

    subtest("get_uuid()")
    {
      struct kc_uuid_t* uuid = new_uuid();
      unsigned char str_uuid[KC_UUID_LENGTH];
      int ret = KC_SUCCESS;

      ret = uuid->get_uuid(uuid, str_uuid);
      ok(ret == KC_SUCCESS);
      ok(strlen((char*)str_uuid) == KC_UUID_LENGTH);

      destroy_uuid(uuid);
    }

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
        { 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 }
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
        { 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 }
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
        { 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 }
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
        { 0x00, 0xc0, 0x4f, 0xd4, 0x30, 0xc8 }
      };

      ok(_check_uuid(namespace_X500));
    }

    // delete the files used to generate the UUID
    struct kc_file_t* file = new_file();

    file->open(file, "nodeid", KC_FILE_OPEN_EXISTING);
    file->delete(file);

    file->open(file, "state", KC_FILE_OPEN_EXISTING);
    file->delete(file);

    destroy_file(file);

    done_testing();
  }

  return 0;
}

