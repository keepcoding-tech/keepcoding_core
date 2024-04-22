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

#include "../datastructs/map.h"

#include <stdio.h>
#include <stdbool.h>

//---------------------------------------------------------------------------//
// -------------------------- HTTP STATUS CODES ---------------------------- //

// --- Informational 1xx --------------------------------------------------- //

#define KC_HTTP_STATUS_100  "100 Continue"
#define KC_HTTP_STATUS_101  "101 Switching Protocols"

// --- Successful 2xx ------------------------------------------------------ //

#define KC_HTTP_STATUS_200  "200 OK"
#define KC_HTTP_STATUS_201  "201 Created"
#define KC_HTTP_STATUS_202  "202 Accepted"
#define KC_HTTP_STATUS_203  "203 Non-Authoritative Information"
#define KC_HTTP_STATUS_204  "204 No Content"
#define KC_HTTP_STATUS_205  "205 Reset Content"
#define KC_HTTP_STATUS_206  "206 Partial Content"

// --- Redirection 3xx ----------------------------------------------------- //

#define KC_HTTP_STATUS_300  "300 Multiple Choices"
#define KC_HTTP_STATUS_301  "301 Moved Permanently"
#define KC_HTTP_STATUS_302  "302 Found"
#define KC_HTTP_STATUS_303  "303 See Other"
#define KC_HTTP_STATUS_304  "304 Not Modified"
#define KC_HTTP_STATUS_305  "305 Use Proxy"
#define KC_HTTP_STATUS_306  "306 (Unused)"
#define KC_HTTP_STATUS_307  "307 Temporary Redirect"

// --- Client Error 4xx ---------------------------------------------------- //

#define KC_HTTP_STATUS_400  "400 Bad Request"
#define KC_HTTP_STATUS_401  "401 Unauthorized"
#define KC_HTTP_STATUS_402  "402 Payment Required"
#define KC_HTTP_STATUS_403  "403 Forbidden"
#define KC_HTTP_STATUS_404  "404 Not Found"
#define KC_HTTP_STATUS_405  "405 Method Not Allowed"
#define KC_HTTP_STATUS_406  "406 Not Acceptable"
#define KC_HTTP_STATUS_407  "407 Proxy Authentication Required"
#define KC_HTTP_STATUS_408  "408 Request Timeout"
#define KC_HTTP_STATUS_409  "409 Conflict"
#define KC_HTTP_STATUS_410  "410 Gone"
#define KC_HTTP_STATUS_411  "411 Length Required"
#define KC_HTTP_STATUS_412  "412 Precondition Failed"
#define KC_HTTP_STATUS_413  "413 Request Entity Too Large"
#define KC_HTTP_STATUS_414  "414 Request-URI Too Long"
#define KC_HTTP_STATUS_415  "415 Unsupported Media Type"
#define KC_HTTP_STATUS_416  "416 Requested Range Not Satisfiable"
#define KC_HTTP_STATUS_417  "417 Expectation Failed"

// --- Server Error 5xx ---------------------------------------------------- //

#define KC_HTTP_STATUS_500  "500 Internal Server Error"
#define KC_HTTP_STATUS_501  "501 Not Implemented"
#define KC_HTTP_STATUS_502  "502 Bad Gateway"
#define KC_HTTP_STATUS_503  "503 Service Unavailable"
#define KC_HTTP_STATUS_504  "504 Gateway Timeout"
#define KC_HTTP_STATUS_505  "505 HTTP Version Not Supported"

//---------------------------------------------------------------------------//
// ------------------------------- HTTP METHODS ---------------------------- //

#define KC_HTTP_METHOD_OPTIONS  "OPTIONS"
#define KC_HTTP_METHOD_GET      "GET"
#define KC_HTTP_METHOD_HEAD     "HEAD"
#define KC_HTTP_METHOD_POST     "POST"
#define KC_HTTP_METHOD_PUT      "PUT"
#define KC_HTTP_METHOD_DELETE   "DELETE"
#define KC_HTTP_METHOD_TRACE    "TRACE"
#define KC_HTTP_METHOD_CONNECT  "CONNECT"

//---------------------------------------------------------------------------//
// ------------------------------- HTTP VERSIONS --------------------------- //

#define KC_HTTP_1 "HTTP/1.1"
#define KC_HTTP_2 "HTTP/2"

//---------------------------------------------------------------------------//

// TODO: use these values to parse the request body content-type
#define KC_BODY_CONTENT_TYPE_JSON  0xF0000010
#define KC_BODY_CONTENT_TYPE_HTML  0xF0000020
#define KC_BODY_CONTENT_TYPE_TEXT  0xF0000040

//---------------------------------------------------------------------------//

#define KC_MAX_HEADER_LENGTH     2048
#define KC_MAX_HEADERS_LIST_SIZE 20

//---------------------------------------------------------------------------//

struct kc_http_header_t
{
  char* key;    // the key of the header (ex: Content-Type)
  char* val;    // the value of the header (ex: text/html)

  size_t size;  // the size of the value
};

struct kc_http_header_t* new_header      (char* key, char* val);
void                     destroy_header  (struct kc_http_header_t* header);

//---------------------------------------------------------------------------//

struct kc_http_request_t
{
  char* method;    // the method to be used (ex: GET, POST, PUT, etc;)
  char* url;       // the endpoint URL (ex: /home/user)
  char* http_ver;  // the HTTP version (ex: HTTP/1.1)
  char* body;      // the HTTP body (ex: JSON, TEXT, HTML, etc;)

  // TODO: add 'param' member to access the parameters

  // the list of headers (ex: Content-Type: text/plain)
  struct kc_map_t* headers;

  int client_fd;  // the client file descriptor

  int (*add_header)     (struct kc_http_request_t* self, char* key, char* val);
  int (*set_method)     (struct kc_http_request_t* self, char* method);
  int (*set_url)        (struct kc_http_request_t* self, char* url);
  int (*set_http_ver)   (struct kc_http_request_t* self, char* http_ver);
  int (*set_body)       (struct kc_http_request_t* self, char* body);
  int (*set_client_fd)  (struct kc_http_request_t* self, int fd);
};

struct kc_http_request_t* new_request      (void);
void                      destroy_request  (struct kc_http_request_t* req);

//---------------------------------------------------------------------------//

struct kc_http_response_t
{
  char* http_ver;     // the HTTP version (ex: HTTP/1.1)
  char* status_code;  // the status code (ex: 200 OK)
  char* body;         // the content of the page

  // the list of headers (ex: Content-Type: text/html)
  struct kc_http_header_t* headers[KC_MAX_HEADERS_LIST_SIZE];
  int headers_len;

  int (*add_header)       (struct kc_http_response_t* self, char* key, char* val);
  int (*set_http_ver)     (struct kc_http_response_t* self, char* http_ver);
  int (*set_status_code)  (struct kc_http_response_t* self, char* status_code);
  int (*set_body)         (struct kc_http_response_t* self, char* body);
};

struct kc_http_response_t* new_response      (void);
void                       destroy_response  (struct kc_http_response_t* res);

//---------------------------------------------------------------------------//

int http_parse_request_line     (char* request_line, struct kc_http_request_t* req);
int http_parse_request_headers  (char* request_headers, struct kc_http_request_t* req);
int http_parse_request_body     (char* request_body, struct kc_http_request_t* req);

//---------------------------------------------------------------------------//

#endif /* KC_HTTP_H */
