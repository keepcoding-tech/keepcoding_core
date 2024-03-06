// This file is part of keepcoding_core
// ==================================
//
// security.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/security/base64.h"
#include "../hdrs/common.h"
#include "../hdrs/test.h"

#include <stdio.h>

int main()
{
  testgroup("Base64")
  {
    subtest("kc_base64_encode()")
    {
      char* out;
      int ret = KC_INVALID;

      const char* empty = "";
      ret = kc_base64_encode(empty, sizeof(char) * strlen(empty), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "") == 0);
      free(out);

      const char* f = "f";
      ret = kc_base64_encode(f, sizeof(char) * strlen(f), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "Zg==") == 0);
      free(out);

      const char* fo = "fo";
      ret = kc_base64_encode(fo, sizeof(char) * strlen(fo), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "Zm8=") == 0);
      free(out);

      const char* foo = "foo";
      ret = kc_base64_encode(foo, sizeof(char) * strlen(foo), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "Zm9v") == 0);
      free(out);

      const char* foob = "foob";
      ret = kc_base64_encode(foob, sizeof(char) * strlen(foob), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "Zm9vYg==") == 0);
      free(out);

      const char* fooba = "fooba";
      ret = kc_base64_encode(fooba, sizeof(char) * strlen(fooba), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "Zm9vYmE=") == 0);
      free(out);

      const char* foobar = "foobar";
      ret = kc_base64_encode(foobar, sizeof(char) * strlen(foobar), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "Zm9vYmFy") == 0);
      free(out);
    }

    subtest("kc_base64_decode()")
    {
      char* out;
      int ret = KC_INVALID;

      const char* empty = "";
      ret = kc_base64_decode(empty, sizeof(char) * strlen(empty), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "") == 0);
      free(out);

      const char* f = "Zg==";
      ret = kc_base64_decode(f, sizeof(char) * strlen(f), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "f") == 0);
      printf("%s\n", out);
      free(out);

      const char* fo = "Zm8=";
      ret = kc_base64_decode(fo, sizeof(char) * strlen(fo), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "fo") == 0);
      printf("%s\n", out);
      free(out);

      const char* foo = "Zm9v";
      ret = kc_base64_decode(foo, sizeof(char) * strlen(foo), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "foo") == 0);
      printf("%s\n", out);
      free(out);

      const char* foob = "Zm9vYg==";
      ret = kc_base64_decode(foob, sizeof(char) * strlen(foob), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "foob") == 0);
      printf("%s\n", out);
      free(out);

      const char* fooba = "Zm9vYmE=";
      ret = kc_base64_decode(fooba, sizeof(char) * strlen(fooba), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "fooba") == 0);
      printf("%s\n", out);
      free(out);

      const char* foobar = "Zm9vYmFy";
      ret = kc_base64_decode(foobar, sizeof(char) * strlen(foobar), &out);
      ok(ret == KC_SUCCESS);
      ok(strcmp(out, "foobar") == 0);
      printf("%s\n", out);
      free(out);
    }

    done_testing();
  }

  return 0;
}