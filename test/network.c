// This file is part of keepcoding_core
// ==================================
//
// network.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/network/server.h"
#include "../hdrs/network/client.h"
#include "../hdrs/test.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  if (strcmp(argv[1], "s") == 0)
  {
    struct kc_server_t* server = new_server("", 8000);
    server->start(server);
    destroy_server(server);
  }
  else
  {
    struct kc_client_t* client = new_client("127.0.0.1", 8000);
    client->start(client);
    destroy_client(client);
  }

  // testgroup("kc_server_t")
  // {
  //   subtest("init/dest")
  //   {
  //     struct kc_server_t* server = new_server("", 8000);
  //     ok(server != NULL);
  //     destroy_server(server);
  //   }

  //   subtest("start & stop")
  //   {
  //     struct kc_server_t* server = new_server("", 8000);

  //     int ret = KC_INVALID;

  //     ret = server->start(server);
  //     ok(ret == KC_SUCCESS);

  //     ret = server->stop(server);
  //     ok(ret == KC_SUCCESS);

  //     destroy_server(server);
  //   }

  //   done_testing();
  // }

  // testgroup("kc_client_t")
  // {
  //   subtest("init/dest")
  //   {
  //     struct kc_client_t* client = new_client("127.0.0.1", 8000);
  //     ok(client != NULL);
  //     destroy_client(client);
  //   }

  //   subtest("start & stop")
  //   {
  //     struct kc_client_t* client = new_client("127.0.0.1", 8000);

  //     int ret = KC_INVALID;

  //     ret = client->start(client);
  //     ok(ret == KC_SUCCESS);

  //     ret = client->stop(client);
  //     ok(ret == KC_SUCCESS);

  //     destroy_client(client);
  //   }

  //   done_testing();
  // }

  return 0;
}
