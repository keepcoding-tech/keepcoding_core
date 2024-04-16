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

//--- MARK: PUBLIC GLOBAL FUNCTION PROTOTYPES -------------------------------//

int http_parse_request_line     (char* request_line, struct kc_http_request_t* req);
int http_parse_request_headers  (char* request_headers, struct kc_http_request_t* req);
int http_parse_request_body     (char* request_body, struct kc_http_request_t* req);

//---------------------------------------------------------------------------//

int http_parse_request_line(char* request_line, struct kc_http_request_t* req)
{
  // make sure the request_line exists
  if (request_line == NULL)
  {
    return KC_FORMAT_ERROR;
  }

  if (req == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // parse the request line
  char* tmp_method   = strtok(request_line, " ");
  char* tmp_url      = strtok(NULL, " ");
  char* tmp_http_ver = strtok(NULL, "\r\n");

  int ret = KC_SUCCESS;

  if (tmp_method == NULL || tmp_url == NULL || tmp_http_ver == NULL)
  {
    return KC_FORMAT_ERROR;
  }

  ret = req->set_method(req, tmp_method);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  ret = req->set_url(req, tmp_url);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  ret = req->set_http_ver(req, tmp_http_ver);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int http_parse_request_headers(char* request_headers, struct kc_http_request_t* req)
{
  // make sure the request_headers exists
  if (request_headers == NULL)
  {
    return KC_FORMAT_ERROR;
  }

  if (req == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // parse the headers
  char* tmp_header = strtok(request_headers, "\r\n");
  while (tmp_header != NULL)
  {
    int i = 0, j = 0;

    // parse the key
    char key[1024];
    while (tmp_header[i] != ':')
    {
      key[i] = tmp_header[i];
      ++i;
    }
    key[i + 1] = '\0';

    // skip the space
    if (tmp_header[++i] == ' ')
    {
      ++i;
    }

    // parse the value
    char val[1024];
    while (tmp_header[i] != '\r')
    {
      val[j++] = tmp_header[i];
      ++i;
    }
    val[j + 1] = '\0';

    // create the header
    if (key[0] != '\0' && val[0] != '\0')
    {
      int ret = req->add_header(req, key, val);
      if (ret != KC_SUCCESS)
      {
        return ret;
      }
    }

    tmp_header = strtok(NULL, "\r\n");
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int http_parse_request_body(char* request_body, struct kc_http_request_t* req)
{
  // make sure the request_body exists
  if (request_body == NULL)
  {
    return KC_FORMAT_ERROR;
  }

  if (req == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  req->set_body(req, request_body);

  return KC_SUCCESS;
}

//--- MARK: PUBLIC HEADER FUNCTION PROTOTYPES -------------------------------//

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
    return;
  }

  if (header->key != NULL)
  {
    free(header->key);
  }

  if (header->val != NULL)
  {
    free(header->val);
  }

  free(header);
}

//--- MARK: PUBLIC REQUEST FUNCTION PROTOTYPES ------------------------------//

int add_req_header     (struct kc_http_request_t* self, char* key, char* val);
int set_req_method     (struct kc_http_request_t* self, char* method);
int set_req_url        (struct kc_http_request_t* self, char* url);
int set_req_http_ver   (struct kc_http_request_t* self, char* http_ver);
int set_req_body       (struct kc_http_request_t* self, char* body);
int set_req_client_fd  (struct kc_http_request_t* self, int fd);

//---------------------------------------------------------------------------//

struct kc_http_request_t* new_request(void)
{
  struct kc_http_request_t* new_req = malloc(sizeof(struct kc_http_request_t));

  // check the allocation of the memory
  if (new_req == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // asign the values
  new_req->set_method    = NULL;
  new_req->set_url       = NULL;
  new_req->set_http_ver  = NULL;
  new_req->set_body      = NULL;
  new_req->headers[0]    = NULL;
  new_req->headers_len   = 0;

  // asign the methods
  new_req->add_header    = add_req_header;
  new_req->set_method    = set_req_method;
  new_req->set_url       = set_req_url;
  new_req->set_http_ver  = set_req_http_ver;
  new_req->set_body      = set_req_body;
  new_req->set_client_fd = set_req_client_fd;

  return new_req;
}

//---------------------------------------------------------------------------//

void destroy_request(struct kc_http_request_t* req)
{
  if (req == NULL)
  {
    return;
  }

  if (req->method != NULL)
  {
    free(req->method);
  }

  if (req->url != NULL)
  {
    free(req->url);
  }

  if (req->http_ver != NULL)
  {
    free(req->http_ver);
  }

  if (req->body != NULL)
  {
    free(req->body);
  }

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
    return KC_NULL_REFERENCE;
  }

  if (self->headers_len + 1 > KC_MAX_HEADERS_LIST_SIZE)
  {
    return KC_INDEX_OUT_OF_BOUNDS;
  }

  // create a new header to be asign
  struct kc_http_header_t* header = new_header(key, val);
  if (header == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // asign the header
  self->headers[self->headers_len] = header;
  self->headers_len++;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int set_req_method(struct kc_http_request_t* self, char* method)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // if the field is being reset, free the memory first
  if (self->method != NULL)
  {
    free(self->method);
  }

  // allocate memory
  self->method = (char*)malloc(sizeof(char) * strlen(method) + 1);
  if (self->method == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // copy the string
  strcpy(self->method, method);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int set_req_url(struct kc_http_request_t* self, char* url)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // if the field is being reset, free the memory first
  if (self->url != NULL)
  {
    free(self->url);
  }

  // allocate memory
  self->url = (char*)malloc(sizeof(char) * strlen(url) + 1);
  if (self->url == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // copy the string
  strcpy(self->url, url);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int set_req_http_ver(struct kc_http_request_t* self, char* http_ver)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // if the field is being reset, free the memory first
  if (self->http_ver != NULL)
  {
    free(self->http_ver);
  }

  // allocate memory
  self->http_ver = (char*)malloc(sizeof(char) * strlen(http_ver) + 1);
  if (self->http_ver == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // copy the string
  strcpy(self->http_ver, http_ver);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int set_req_body(struct kc_http_request_t* self, char* body)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // if the field is being reset, free the memory first
  if (self->body != NULL)
  {
    free(self->body);
  }

  // allocate memory
  self->body = (char*)malloc(sizeof(char) * strlen(body) + 1);
  if (self->body == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // copy the string
  strcpy(self->body, body);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int set_req_client_fd(struct kc_http_request_t* self, int fd)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  self->client_fd = fd;

  return KC_SUCCESS;
}

//--- MARK: PUBLIC RESPONSE FUNCTION PROTOTYPES -----------------------------//

int add_res_header       (struct kc_http_response_t* self, char* key, char* val);
int set_res_http_ver     (struct kc_http_response_t* self, char* http_ver);
int set_res_status_code  (struct kc_http_response_t* self, char* status_code);
int set_res_body         (struct kc_http_response_t* self, char* body);

//---------------------------------------------------------------------------//

struct kc_http_response_t* new_response(void)
{
  struct kc_http_response_t* new_res = malloc(sizeof(struct kc_http_response_t));

  // check the allocation of the memory
  if (new_res == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // asign the values
  new_res->http_ver    = NULL;
  new_res->status_code = NULL;
  new_res->body        = NULL;
  new_res->headers[0]  = NULL;
  new_res->headers_len = 0;

  // asign the methods
  new_res->add_header      = add_res_header;
  new_res->set_http_ver    = set_res_http_ver;
  new_res->set_status_code = set_res_status_code;
  new_res->set_body        = set_res_body;

  return new_res;
}

//---------------------------------------------------------------------------//

void destroy_response(struct kc_http_response_t* res)
{
  if (res == NULL)
  {
    return;
  }

  if (res->http_ver != NULL)
  {
    free(res->http_ver);
  }

  if (res->http_ver != NULL)
  {
    free(res->status_code);
  }

  if (res->http_ver != NULL)
  {
    free(res->body);
  }

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
    return KC_NULL_REFERENCE;
  }

  // search for the header
  for (int i = 0; i < self->headers_len; ++i)
  {
    if (strcmp(self->headers[i]->key, key) == 0)
    {
      // free the header first
      destroy_header(self->headers[i]);

      // create a new header
      self->headers[i] = new_header(key, val);
      if (self->headers[i] == NULL)
      {
        return KC_OUT_OF_MEMORY;
      }

      return KC_SUCCESS;
    }
  }

  // create a new header to be asign
  struct kc_http_header_t* header = new_header(key, val);
  if (header == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // asign the header
  self->headers[self->headers_len] = header;
  self->headers_len++;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int set_res_http_ver(struct kc_http_response_t* self, char* http_ver)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // if the field is being reset, free the memory first
  if (self->http_ver != NULL)
  {
    free(self->http_ver);
  }

  // allocate memory
  self->http_ver = (char*)malloc(sizeof(char) * strlen(http_ver) + 1);
  if (self->http_ver == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // copy the string
  strcpy(self->http_ver, http_ver);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int set_res_status_code(struct kc_http_response_t* self, char* status_code)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // if the field is being reset, free the memory first
  if (self->status_code != NULL)
  {
    free(self->status_code);
  }

  // allocate memory
  self->status_code = (char*)malloc(sizeof(char) * strlen(status_code) + 1);
  if (self->status_code == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // copy the string
  strcpy(self->status_code, status_code);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int set_res_body(struct kc_http_response_t* self, char* body)
{
  if (self == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // if the field is being reset, free the memory first
  if (self->body != NULL)
  {
    free(self->body);
  }

  // allocate memory
  self->body = (char*)malloc(sizeof(char) * strlen(body) + 3);
  if (self->body == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // copy the string
  strcpy(self->body, body);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

