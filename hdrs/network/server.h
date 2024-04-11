// This file is part of keepcoding_core
// ==================================
//
// server.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * a network struct
 */

#ifndef KC_SERVER_T_H
#define KC_SERVER_T_H

#include "../system/logger.h"
#include "socket.h"

#include <stdio.h>
#include <stdbool.h>

//---------------------------------------------------------------------------//

#define KC_SERVER_LOG_PATH "build/log/server.log"

#define KC_SERVER_MAX_CONNECTIONS 1024

#define KC_GET_METHOD    "GET"
#define KC_POST_METHOD   "POST"
#define KC_PUT_METHOD    "PUT"
#define KC_DELETE_METHOD "DELETE"

//---------------------------------------------------------------------------//

struct kc_header_t
{
  char* key;    // the key of the header (ex: Content-Type)
  char* val;    // the value of the header (ex: text/html)
  size_t size;  // the size of the value
};

struct kc_header_t* new_header     (char* key, char* val);
void                destroy_header (struct kc_header_t* header);

//---------------------------------------------------------------------------//

struct kc_request_t
{
  char* method;    // the method to be used (ex: GET, POST, PUT, etc;)
  char* url;       // the endpoint URL (ex: /home/user)
  char* tcp_vers;  // the TCP version (ex: HTTP/1.1)

  // the list of headers (ex: Content-Type: text/plain)
  struct kc_header_t* headers[1024];
  int headers_len;

  int (*add_header)  (struct kc_request_t* self, char* key, char* val);
};

struct kc_request_t* new_request     (char* method, char* url, char* tcp_vers);
void                 destroy_request (struct kc_request_t* req);

//---------------------------------------------------------------------------//

struct kc_response_t
{
  char* tcp_vers;       // the TCP version (ex: HTTP/1.1)
  char* status_code;    // the status code (ex: 200 OK)
  unsigned char* body;  // the content of the page

  // the list of headers (ex: Content-Type: text/plain)
  struct kc_header_t* headers[1024];
  int headers_len;

  int (*add_header)  (struct kc_response_t* self, char* key, char* val);
};

struct kc_response_t* new_response     (char* tcp_vers, char* status_code, unsigned char* body);
void                  destroy_response (struct kc_response_t* res);

//---------------------------------------------------------------------------//

struct kc_route_t
{
  int (*get)     (unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res));
  int (*post)    (unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res));
  int (*put)     (unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res));
  int (*delete)  (unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res));
};

//---------------------------------------------------------------------------//

struct kc_server_t
{
  struct kc_socket_t* socket;  // server' socket
  struct kc_route_t*  routes;  // server' endpoints

  int (*start)   (struct kc_server_t* self);
};

struct kc_server_t* new_server_IPv4  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server_IPv6  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server       (const int AF, const char* IP, const unsigned int PORT);
void                destroy_server   (struct kc_server_t* server);

//---------------------------------------------------------------------------//

#endif /* KC_SERVER_T_H */
