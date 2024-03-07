// This file is part of keepcoding_core
// ==================================
//
// security.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/security/base64.h"
#include "../hdrs/security/uuid.h"
#include "../hdrs/common.h"
#include "../hdrs/test.h"

#include <stdio.h>
#include <string.h>

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

int main()
{
  testgroup("Base64")
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

  testgroup("UUID")
  {
    subtest("kc_generate_uuid")
    {
      char* uuid;
      int ret = kc_generate_uuid(&uuid);
      printf("\n%s\n", uuid);
      ok(ret == KC_SUCCESS);
    }

    done_testing();
  }

  return 0;
}
