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

#include "http.h"
#include "socket.h"

#include <stdio.h>
#include <stdbool.h>

//---------------------------------------------------------------------------//

#define KC_SERVER_LOG_PATH "build/log/server.log"

#define KC_SERVER_MAX_CONNECTIONS 1024
#define KC_REQUEST_MAX_SIZE       2048

#define KC_GET_METHOD    "GET"
#define KC_POST_METHOD   "POST"
#define KC_PUT_METHOD    "PUT"
#define KC_DELETE_METHOD "DELETE"

//---------------------------------------------------------------------------//

struct kc_route_t
{
  int (*get)     (unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res));
  int (*post)    (unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res));
  int (*put)     (unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res));
  int (*delete)  (unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res));
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
