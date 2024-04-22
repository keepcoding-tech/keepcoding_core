// This file is part of keepcoding_core
// ==================================
//
// map.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/datastructs/map.h"
#include "../hdrs/network/server.h"
#include "../hdrs/common.h"
#include "../hdrs/test.h"

#include <stdio.h>
#include <string.h>

void map_dump(struct kc_map_t* map)
{
  for (int i = 0; i < KC_MAP_MAX_SIZE; ++i)
  {
    struct kc_entry_t* entry = map->entries[i];

    if (entry == NULL)
    {
      continue;
    }

    printf("slot[%d]: ", i);

    for (;;)
    {
      struct kc_endpoint_t* endpoint = (struct kc_endpoint_t*)(entry->val);
      printf("url: %s - method: %s, ", endpoint->url, endpoint->method);

      if (entry->next == NULL)
      {
        break;
      }

      entry = entry->next;
    }

    printf("\n");
  }
}

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

    subtest("hash()")
    {
      struct kc_map_t* map = new_map();

      int ret = KC_INVALID;

      struct kc_endpoint_t* get_home = new_endpoint("GET", "/");
      ret = map->set(map, "/", get_home, sizeof(struct kc_endpoint_t));
      ok(ret == KC_SUCCESS);

      struct kc_endpoint_t* get_user = new_endpoint("GET", "/user");
      ret = map->set(map, "/user", get_user, sizeof(struct kc_endpoint_t));
      ok(ret == KC_SUCCESS);

      struct kc_endpoint_t* add_user = new_endpoint("POST", "/create/user");
      ret = map->set(map, "/create/user", add_user, sizeof(struct kc_endpoint_t));
      ok(ret == KC_SUCCESS);

      map_dump(map);


      destroy_map(map);
    }

    done_testing();
  }
  return 0;
}
