// This file is part of keepcoding_core
// ==================================
//
// http.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * a network struct
 */

#ifndef KC_HTTP_H
#define KC_HTTP_H

#include <stdio.h>
#include <stdbool.h>

//---------------------------------------------------------------------------//

#define KC_MAX_HEADER_LENGTH 1024

//---------------------------------------------------------------------------//

struct kc_http_header_t
{
  char* key;    // the key of the header (ex: Content-Type)
  char* val;    // the value of the header (ex: text/html)
  size_t size;  // the size of the value
};

struct kc_http_header_t* new_header  (char* key, char* val);
void destroy_header  (struct kc_http_header_t* header);

//---------------------------------------------------------------------------//

struct kc_http_request_t
{
  char* method;    // the method to be used (ex: GET, POST, PUT, etc;)
  char* url;       // the endpoint URL (ex: /home/user)
  char* http_ver;  // the HTTP version (ex: HTTP/1.1)

  // the list of headers (ex: Content-Type: text/plain)
  struct kc_http_header_t* headers[KC_MAX_HEADER_LENGTH];
  int headers_len;

  int (*add_header)  (struct kc_http_request_t* self, char* key, char* val);
};

struct kc_http_request_t* new_request  (char* method, char* url, char* http_vers);
void destroy_request  (struct kc_http_request_t* req);

//---------------------------------------------------------------------------//

struct kc_http_response_t
{
  char* http_vers;    // the HTTP version (ex: HTTP/1.1)
  char* status_code;  // the status code (ex: 200 OK)
  char* body;         // the content of the page

  // the list of headers (ex: Content-Type: text/html)
  struct kc_http_header_t* headers[KC_MAX_HEADER_LENGTH];
  int headers_len;

  int (*add_header)  (struct kc_http_response_t* self, char* key, char* val);
};

struct kc_http_response_t* new_response  (char* http_vers, char* status_code, char* body);
void destroy_response  (struct kc_http_response_t* res);

//---------------------------------------------------------------------------//

#endif /* KC_HTTP_H */
