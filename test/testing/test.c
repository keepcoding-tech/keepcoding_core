// This file is part of keepcoding_core
// ==================================
//
// test.c
//
// Copyright (c) 2023 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/testing/test.h"

#include <stdio.h>
#include <string.h>

int main() {
  testgroup("This is a test group")
  {
    subtest("subtest test XD")
    {
      // should pass
      ok(1 + 1 == 2)

      note("the next test should fail")
      //ok(3 * 2 == 7)

      // should pass
      ok(strcmp("same", "text") != 0)

      // should do nothing (skip
      skip(strcmp("same", "text") == 0)
    }

    subtest("second subtest test XD")
    {
      int arr[] = {1, 2, 3, 4, 5};
      note("the first 3 tests will be skiped")
      for (int i = 0; i < 5; ++i)
      {
        skip(i < 3)
        {
          ok(arr[i] == i + 1)
        }
      }
    }

    done_testing()
  }

  return 0;
}
