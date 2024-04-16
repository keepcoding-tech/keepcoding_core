// This file is part of keepcoding_core
// ==================================
//
// http.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/system/logger.h"
#include "../../hdrs/network/http.h"
#include "../../hdrs/common.h"

#include <stdlib.h>
#include <string.h>

//--- MARK: PUBLIC HEADER FUNCTION PROTOTYPES -------------------------------//

struct kc_http_header_t* new_header  (char* key, char* val);
void destroy_header  (struct kc_http_header_t* header);

//--- MARK: PUBLIC REQUEST FUNCTION PROTOTYPES ------------------------------//

struct kc_http_request_t* new_request  (char* method, char* url, char* tcp_vers);
void destroy_request   (struct kc_http_request_t* req);

int add_req_header  (struct kc_http_request_t* self, char* key, char* val);

//--- MARK: PUBLIC RESPONSE FUNCTION PROTOTYPES -----------------------------//

struct kc_http_response_t* new_response  (char* tcp_vers, char* status_code, char* body);
void destroy_response  (struct kc_http_response_t* res);

int add_res_header  (struct kc_http_response_t* self, char* key, char* val);

//---------------------------------------------------------------------------//

struct kc_http_header_t* new_header(char* key, char* val)
{
  // create a new instance to be returned
  struct kc_http_header_t* new_header = malloc(sizeof(struct kc_http_header_t));

  // check the allocation of the memory
  if (new_header == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // allocate memory for the key
  new_header->key = (char*)malloc(sizeof(char) * strlen(key) + 1);
  if (new_header->key == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_header);

    return NULL;
  }

  // allocate memory for the value
  new_header->val = (char*)malloc(sizeof(char) * strlen(val) + 1);
  if (new_header->val == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_header->key);
    free(new_header);

    return NULL;
  }

  // asign the values
  strcpy(new_header->key, key);
  strcpy(new_header->val, val);
  new_header->size = strlen(val);

  return new_header;
}

//---------------------------------------------------------------------------//

void destroy_header(struct kc_http_header_t* header)
{
  if (header == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(header->key);
  free(header->val);
  free(header);
}

//---------------------------------------------------------------------------//

struct kc_http_request_t* new_request(char* method, char* url, char* http_vers)
{
  struct kc_http_request_t* new_req = malloc(sizeof(struct kc_http_request_t));

  // check the allocation of the memory
  if (new_req == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // allocate memory for the method
  new_req->method = (char*)malloc(sizeof(char) * strlen(method) + 1);
  if (new_req->method == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_req);

    return NULL;
  }

  // allocate memory for the url
  new_req->url = (char*)malloc(sizeof(char) * strlen(url) + 1);
  if (new_req->url == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_req->method);
    free(new_req);

    return NULL;
  }

  // allocate memory for the tcp_version
  new_req->http_ver = (char*)malloc(sizeof(char) * strlen(http_vers) + 1);
  if (new_req->http_ver == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_req->method);
    free(new_req->url);
    free(new_req);

    return NULL;
  }

  // asign the values
  strcpy(new_req->method, method);
  strcpy(new_req->url, url);
  strcpy(new_req->http_ver, http_vers);
  new_req->headers_len = 0;

  // asign the methods
  new_req->add_header = add_req_header;

  return new_req;
}

//---------------------------------------------------------------------------//

void destroy_request(struct kc_http_request_t* req)
{
  if (req == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(req->method);
  free(req->url);
  free(req->http_ver);

  for (int i = 0; i < req->headers_len; ++i)
  {
    destroy_header(req->headers[i]);
  }

  free(req);
}

//---------------------------------------------------------------------------//

int add_req_header(struct kc_http_request_t* self, char* key, char* val)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // create a new header to be asign
  struct kc_http_header_t* header = new_header(key, val);
  if (header == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return KC_OUT_OF_MEMORY;
  }

  // asign the header
  self->headers[self->headers_len] = header;
  self->headers_len++;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

struct kc_http_response_t* new_response(char* http_vers, char* status_code, char* body)
{
  struct kc_http_response_t* new_res = malloc(sizeof(struct kc_http_response_t));

  // check the allocation of the memory
  if (new_res == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // allocate memory for the tcp_vers
  new_res->http_vers = (char*)malloc(sizeof(char) * strlen(http_vers) + 1);
  if (new_res->http_vers == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_res);

    return NULL;
  }

  // allocate memory for the status_code
  new_res->status_code = (char*)malloc(sizeof(char) * strlen(status_code) + 1);
  if (new_res->status_code == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_res->http_vers);
    free(new_res);

    return NULL;
  }

  // allocate memory for the url
  new_res->body = (char*)malloc(sizeof(char) * strlen(body) + 1);
  if (new_res->body == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_res->http_vers);
    free(new_res->status_code);
    free(new_res);

    return NULL;
  }

  // asign the values
  strcpy(new_res->http_vers, http_vers);
  strcpy(new_res->status_code, status_code);
  strcpy(new_res->body, body);
  new_res->headers_len = 0;

  // asign the methods
  new_res->add_header = add_res_header;

  return new_res;
}

//---------------------------------------------------------------------------//

void destroy_response (struct kc_http_response_t* res)
{
  if (res == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(res->http_vers);
  free(res->status_code);
  free(res->body);

  for (int i = 0; i < res->headers_len; ++i)
  {
    destroy_header(res->headers[i]);
  }

  free(res);
}

//---------------------------------------------------------------------------//

int add_res_header(struct kc_http_response_t* self, char* key, char* val)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // create a new header to be asign
  struct kc_http_header_t* header = new_header(key, val);
  if (header == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return KC_OUT_OF_MEMORY;
  }

  // asign the header
  self->headers[self->headers_len] = header;
  self->headers_len++;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
