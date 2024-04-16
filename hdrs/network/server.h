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

#define KC_SERVER_MAX_CONNECTIONS  1024
#define KC_REQUEST_MAX_SIZE        2048
#define KC_RESPONSE_MAX_SIZE       2048

//---------------------------------------------------------------------------//

#define KC_SERVER_SEND_MSG  0xF0000010
#define KC_SERVER_RENDER    0xF0000020
#define KC_SERVER_REDIRECT  0xF0000040

//---------------------------------------------------------------------------//

struct kc_endpoint_t;
struct kc_route_t;
struct kc_server_t;

//---------------------------------------------------------------------------//

struct kc_endpoint_t
{
  char* method;    // the method to be used (ex: GET, POST, PUT, etc;)
  char* url;       // the endpoint URL (ex: /home/user)

  // the callback function to get called
  int (*callback)  (struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res);
};

struct kc_endpoint_t* new_endpoint      (char* method, char* url);
void                  destroy_endpoint  (struct kc_endpoint_t* endpoint);

//---------------------------------------------------------------------------//

struct kc_route_t
{
  void (*options)  (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
  void (*get)      (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
  void (*head)     (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
  void (*post)     (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
  void (*put)      (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
  void (*delete)   (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
  void (*trace)    (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
  void (*connect)  (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
};

//---------------------------------------------------------------------------//

struct kc_server_t
{
  struct kc_socket_t* socket;  // server' socket
  struct kc_route_t*  routes;  // server' endpoints

  int (*start)  (struct kc_server_t* self);
  int (*send)   (int client_fd, struct kc_http_response_t* res);

  // TODO: add stop server function
};

struct kc_server_t* new_server_IPv4  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server_IPv6  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server       (const int AF, const char* IP, const unsigned int PORT);
void                destroy_server   (struct kc_server_t* server);
int                 start_server     (struct kc_server_t* self);
int                 send_msg_server  (int client_fd, struct kc_http_response_t* res);

//---------------------------------------------------------------------------//

#endif /* KC_SERVER_T_H */
