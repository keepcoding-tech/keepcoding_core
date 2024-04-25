// This file is part of keepcoding_core
// ==================================
//
// network.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../hdrs/datastructs/map.h"
#include "../hdrs/network/server.h"
#include "../hdrs/network/client.h"
#include "../hdrs/network/http.h"
#include "../hdrs/network/http_parser.h"
#include "../hdrs/test.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

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
  printf("HTTP version: %s \n\n", req->http_ver);

  for (int i = 0; i < KC_MAP_MAX_SIZE; ++i)
  {
    struct kc_entry_t* entry = req->headers->entries[i];

    if (entry == NULL)
    {
      continue;
    }

    for (;;)
    {
      printf("%s: %s \n", (char*)(entry->key), (char*)(entry->val));

      if (entry->next == NULL)
      {
        break;
      }

      entry = entry->next;
    }
  }

  printf("\n\n");

  char* header_val = NULL;
  int ret = req->headers->get(req->headers, "Host", (void*)&header_val);
  if (ret != KC_SUCCESS)
  {
    printf("%d", ret);
  }

  printf("\n\n%s\n\n", header_val);

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
  // this will stop the logger from displaing
  // the logs to the console while testing
  logger_debug_mode = true;

  testgroup("kc_socket_t")
  {
    subtest("init/desc")
    {
      // create an IPv4 to test it
      struct kc_socket_t* socket_ip4 = new_socket(AF_INET, "0.0.0.0", 8000);

      ok(socket_ip4 != NULL);
      ok(socket_ip4->fd > 0);
      ok(socket_ip4->addr != NULL);
      ok(strcmp(socket_ip4->ip, "0.0.0.0") == 0);
      ok(socket_ip4->port == 8000);

      destroy_socket(socket_ip4);

      // create an IPv6 to test it
      struct kc_socket_t* socket_ip6 =
          new_socket(AF_INET6, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", 8000);

      ok(socket_ip6 != NULL);
      ok(socket_ip6->fd > 0);
      ok(socket_ip6->addr != NULL);
      ok(strcmp(socket_ip6->ip, "32.1.13.184") == 0);
      ok(socket_ip6->port == 8000);

      destroy_socket(socket_ip6);
    }

    subtest("_validate_socket_ip()")
    {
      note("IPv4 octets > 255");
      ok(new_socket(AF_INET, "256.0.0.1", 8000) == NULL);

      note("IPv6 double ::");
      ok(new_socket(AF_INET6, "2001:db8:1::1::1", 8000) == NULL);

      note("IPv6 as IPv4");
      ok(new_socket(AF_INET, "2001:db8:85a3::8a2e:370:7334", 8000) == NULL);

      note("IPv4 as IPv6");
      ok(new_socket(AF_INET6, "192.168.0.11", 8000) == NULL);

      note("IPv4 or IPv6 as string");
      ok(new_socket(AF_INET, "just.a::string", 8000) == NULL);
    }

    subtest("_validate_socket_port()")
    {
      note("PORT < 0");
      ok(new_socket(AF_INET, "0.0.0.0", -1) == NULL);

      note("0 < PORT < 1024");
      ok(new_socket(AF_INET, "0.0.0.0", 512) == NULL);

      note("49151 < PORT < 65535");
      ok(new_socket(AF_INET, "0.0.0.0", 55555) == NULL);

      note("65535 < PORT");
      ok(new_socket(AF_INET, "0.0.0.0", 75555) == NULL);
    }

    done_testing();
  }

  testgroup("http_parser")
  {
    subtest("http_parse_request_line()")
    {

    }

    subtest("http_parse_request_headers()")
    {

    }

    subtest("http_parse_request_body()")
    {

    }

    subtest("validate_http_method()")
    {
      const char* valid_methods[] =
      {
        "OPTIONS", "GET", "HEAD", "POST",
        "PUT", "DELETE", "TRACE", "CONNECT"
      };

      size_t valid_methods_len =
          sizeof(valid_methods) / sizeof(valid_methods[0]);
      for (size_t i = 0; i < valid_methods_len; ++i)
      {
        ok(validate_http_method((char*)valid_methods[i])== KC_SUCCESS);
      }

      ok(validate_http_method("RANDOM_STRING") == KC_INVALID);
      ok(validate_http_method(NULL) == KC_NULL_REFERENCE);
    }

    subtest("validate_http_url()")
    {
      const char *valid_paths[] =
      {
        "/validpath", "/path_with_underscore", "/path-with-hyphen",
        "/valid/path", "/path_/_with_/_underscore", "/path-/-with-/-hyphen",
        "/path123/with456/nums789/0", "/0/1/2/3/4/5/6/7/8/9/", "/-_/_-", "/",
        "/ABCDEFGHIJKLMNOPQRSTUVWXYZ", "/abcdefghijklmnopqrstuvwxyz",
        "/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
        "/0123456789"
      };

      // test valid paths
      size_t valid_paths_len = sizeof(valid_paths) / sizeof(valid_paths[0]);
      for (size_t i = 0; i < valid_paths_len; ++i)
      {
        int ret = validate_http_url((char*)valid_paths[i]);
        ok(ret == KC_SUCCESS);
      }

      const char *invalid_paths[] =
      {
        "/path with space", "/path\twith\ttab", "/path\nwith\nnewline",
        "/path\rwith\rcarriage\rreturn", "/path\fwith\fform\ffeed",
        "/path\vwith\vvertical\vtab", "/path\bwith\bback\bspace",
        "/path?", "/path#", "/path[", "/path]", "/path@", "/path:",
        "/path\\", "/path\"", "/path<", "/path>",
        "/path{", "/path}", "/path|", "/path^", "/path`", "/path!",
        "/path*", "/path(", "/path)", "/path{", "/path}",
        "/path ", "/path\b", "/path\x7F", "/path\x80", "/path\xFF"
      };

      // test invalid paths
      size_t invalid_paths_len = sizeof(invalid_paths) / sizeof(invalid_paths[0]);
      for (size_t i = 0; i < invalid_paths_len; ++i)
      {
        int ret = validate_http_url((char*)invalid_paths[i]);
        ok(ret == KC_INVALID);
      }

      ok(validate_http_url(NULL) == KC_NULL_REFERENCE);
    }

    subtest("validate_http_ver()")
    {
      // valid HTTP versions
      ok(validate_http_ver("HTTP/1.1") == KC_SUCCESS);
      ok(validate_http_ver("HTTP/2") == KC_SUCCESS);

      // invalid HTTP versions
      ok(validate_http_ver("HTTP/3") == KC_INVALID);
      ok(validate_http_ver(NULL) == KC_NULL_REFERENCE);
    }

    subtest("validate_http_body()")
    {
      // valid body
      ok(validate_http_body("<h1>Hello, World!</h1>") == KC_SUCCESS);

      // invalid body
      ok(validate_http_body(NULL) == KC_NULL_REFERENCE);
    }

    done_testing();
  }

  testgroup("kc_http_request_t")
  {
    subtest("init/desc")
    {
      struct kc_http_request_t* req = new_request();

      ok(req->method == NULL);
      ok(req->url == NULL);
      ok(req->http_ver == NULL);
      ok(req->body == NULL);
      ok(req->client_fd == 0);
      ok(req->params != NULL);
      ok(req->headers != NULL);

      ok(req != NULL);
      ok(req->get_header != NULL);
      ok(req->get_param != NULL);

      destroy_request(req);
    }

    subtest("get_header()")
    {
      struct kc_http_request_t* req = new_request();
      destroy_request(req);
    }

    subtest("get_param()")
    {
      struct kc_http_request_t* req = new_request();
      destroy_request(req);
    }

    done_testing();
  }

  testgroup("kc_http_response_t")
  {
    subtest("init/desc")
    {

    }

    done_testing();
  }

  // testgroup("kc_server_t")
  // {
  //   subtest("init/dest")
  //   {
  //     struct kc_server_t* server = new_server_IPv4("127.0.0.1", 8000);

  //     ok(server != NULL);
  //     ok(server->socket != NULL);
  //     ok(server->routes != NULL);

  //     destroy_server(server);
  //   }

  //   subtest("start()")
  //   {
  //     struct kc_server_t* server = new_server_IPv4("127.0.0.1", 8000);

  //     // add routes
  //     server->routes->get("/get-test", get_test);
  //     server->routes->post("/post-test", post_test);

  //     // start the server
  //     int ret = server->start(server);
  //     ok(ret == KC_SUCCESS);
  //     log_error(kc_error_msg[ret]);

  //     destroy_server(server);
  //   }

  //   done_testing();
  // }

  // testgroup("kc_client_t")
  // {
  //   subtest("init/dest")
  //   {

  //   }

  //   subtest("start()")
  //   {
  //     // TODO: add client tests
  //   }

  //   done_testing();
  // }

  return 0;
}
