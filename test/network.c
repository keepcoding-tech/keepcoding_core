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

void test_server(void)
{
  // create a server instance
  struct kc_server_t* server = new_server_IPv4("0.0.0.0", 8000);

  // start the server
  server->start(server);

  // destroy the server when done using
  destroy_server(server);
}

void test_client(void)
{
  // create a client instance
  struct kc_client_t* client = new_client_IPv4("0.0.0.0", 8000);

  // start the client
  client->start(client);

  // destroy the client when done using
  destroy_client(client);
}

int get_test(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res)
{
  printf("method: %s \n", req->method);
  printf("url: %s \n", req->url);
  printf("HTTP version: %s \n", req->http_ver);

  printf("headers: \n");
  for (int i = 0; i < req->headers_len; ++i)
  {
    printf("%s: %s \n", req->headers[i]->key, req->headers[i]->val);
  }
  printf("\n\n");

  res->set_body(res, "GET test");

  return self->send(req->client_fd, res);
}

int post_test(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res)
{
  res->set_body(res, "POST test");
  return self->send(req->client_fd, res);
}


int main(int argc, char **argv)
{
  // if (argv[1] != NULL)
  // {
  //   if (strcmp(argv[1], "s") == 0)
  //   {
  //     test_server();
  //   }

  //   if (strcmp(argv[1], "c") == 0)
  //   {
  //     test_client();
  //   }
  // }

   testgroup("kc_server_t")
   {
  //   subtest("init/dest")
  //   {
  //     note("valid PORT");
  //     ok(new_server_IPv4("0.0.0.0", 8000) != NULL);

  //     note("PORT < 0");
  //     ok(new_server_IPv4("0.0.0.0", -1) == NULL);

  //     note("0 < PORT < 1024");
  //     ok(new_server_IPv4("0.0.0.0", 512) == NULL);

  //     note("49151 < PORT < 65535");
  //     ok(new_server_IPv4("0.0.0.0", 55555) == NULL);

  //     note("65535 < PORT");
  //     ok(new_server_IPv4("0.0.0.0", 75555) == NULL);

  //     note("valid IPv4");
  //     ok(new_server_IPv4("0.0.0.0", 8000) != NULL);

  //     note("valid IPv6");
  //     ok(new_server_IPv6("2001:0db8:85a3:0000:0000:8a2e:0370:7334", 8000) != NULL);

  //     note("IPv4 octets > 255");
  //     ok(new_server_IPv4("256.0.0.1", 8000) == NULL);

  //     note("IPv6 double ::");
  //     ok(new_server_IPv6("2001:db8:1::1::1", 8000) == NULL);

  //     note("IPv6 as IPv4");
  //     ok(new_server_IPv4("2001:db8:85a3::8a2e:370:7334", 8000) == NULL);

  //     note("IPv4 as IPv6");
  //     ok(new_server_IPv6("192.168.0.11", 8000) == NULL);

  //     note("IPv4 or IPv6 as string");
  //     ok(new_server_IPv4("just.a::string", 8000) == NULL);
  //   }

    subtest("start()")
    {
      struct kc_server_t* server = new_server_IPv4("127.0.0.1", 8000);

      // add routes
      server->routes->get("/get-test", get_test);
      server->routes->post("/post-test", post_test);

      // start the server
      int ret = server->start(server);
      ok(ret == KC_SUCCESS);
      log_error(kc_error_msg[ret]);

      destroy_server(server);
    }

    done_testing();
  }

  // testgroup("kc_client_t")
  // {
  //   subtest("init/dest")
  //   {
  //     note("valid PORT");
  //     ok(new_client_IPv4("0.0.0.0", 8000) != NULL);

  //     note("PORT < 0");
  //     ok(new_client_IPv4("0.0.0.0", -1) == NULL);

  //     note("0 < PORT < 1024");
  //     ok(new_client_IPv4("0.0.0.0", 512) == NULL);

  //     note("49151 < PORT < 65535");
  //     ok(new_client_IPv4("0.0.0.0", 55555) == NULL);

  //     note("65535 < PORT");
  //     ok(new_client_IPv4("0.0.0.0", 75555) == NULL);

  //     note("valid IPv4");
  //     ok(new_client_IPv4("0.0.0.0", 8000) != NULL);

  //     note("valid IPv6");
  //     ok(new_client_IPv6("2001:0db8:85a3:0000:0000:8a2e:0370:7334", 8000) != NULL);

  //     note("IPv4 octets > 255");
  //     ok(new_client_IPv4("256.0.0.1", 8000) == NULL);

  //     note("IPv6 double ::");
  //     ok(new_client_IPv6("2001:db8:1::1::1", 8000) == NULL);

  //     note("IPv6 as IPv4");
  //     ok(new_client_IPv4("2001:db8:85a3::8a2e:370:7334", 8000) == NULL);

  //     note("IPv4 as IPv6");
  //     ok(new_client_IPv6("192.168.0.11", 8000) == NULL);

  //     note("IPv4 or IPv6 as string");
  //     ok(new_client_IPv4("just.a::string", 8000) == NULL);
  //   }

  //   subtest("start()")
  //   {
  //     //struct kc_client_t* client = new_client_IPv4("0.0.0.0", 8000);
  //     //int ret = KC_SUCCESS;

  //     //ret = client->start(client);
  //     //ok(ret == KC_SUCCESS);

  //     //destroy_client(client);
  //   }

  //   done_testing();
  // }

  // TODO: add tests for http

  return 0;
}
