// This file is part of keepcoding_core
// ==================================
//
// map.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/datastructs/map.h"
#include "../hdrs/common.h"
#include "../hdrs/test.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
  testgroup("kc_map_t")
  {
    subtest("init/desc")
    {
      struct kc_map_t* map = new_map();
      ok(map != NULL);
      destroy_map(map);
    }

    subtest("set()")
    {
      struct kc_map_t* map = new_map();

      destroy_map(map);
    }

    subtest("get()")
    {
      struct kc_map_t* map = new_map();

      destroy_map(map);
    }

    done_testing();
  }
  return 0;
}
