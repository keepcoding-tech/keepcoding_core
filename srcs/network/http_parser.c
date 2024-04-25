// This file is part of keepcoding_core
// ==================================
//
// http_parser.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/network/http_parser.h"
#include "../../hdrs/common.h"

#include <stdlib.h>
#include <string.h>

//--- MARK: PUBLIC GLOBAL FUNCTION PROTOTYPES -------------------------------//

int http_parse_request_line     (char* request_line, struct kc_http_request_t* req);
int http_parse_request_headers  (char* request_headers, struct kc_http_request_t* req);
int http_parse_request_body     (char* request_body, struct kc_http_request_t* req);
int validate_http_method        (char* method);
int validate_http_url           (char* url);
int validate_http_ver           (char* http_ver);
int validate_http_body          (char* body);

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

  ret = _set_req_method(req, tmp_method);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  ret = _set_req_url(req, tmp_url);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  ret = _set_req_http_ver(req, tmp_http_ver);
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
      // the map will handle dublicates and everything else
      int ret = req->headers->set(req->headers, key,
          val, (sizeof(char) * strlen(val) + 1));

      // make sure the insertion was made
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

  _set_req_body(req, request_body);

  return KC_SUCCESS;
}

int validate_http_method(char* method)
{
  // make sure the method exists
  if (method == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // the must be an existing valid method
  if (strcmp(method, KC_HTTP_METHOD_OPTIONS) != 0 &&
      strcmp(method, KC_HTTP_METHOD_GET)     != 0 &&
      strcmp(method, KC_HTTP_METHOD_HEAD)    != 0 &&
      strcmp(method, KC_HTTP_METHOD_POST)    != 0 &&
      strcmp(method, KC_HTTP_METHOD_PUT)     != 0 &&
      strcmp(method, KC_HTTP_METHOD_DELETE)  != 0 &&
      strcmp(method, KC_HTTP_METHOD_TRACE)   != 0 &&
      strcmp(method, KC_HTTP_METHOD_CONNECT) != 0)
  {
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int validate_http_url(char* url)
{
  // make sure the url exists
  if (url == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // reserved characters
  // TODO: use encoding characters too .-_~!$&\'()*+,;=:@
  const char valid_chars[66] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789-_/";

  const int CONTROL_CHAR   = 32;
  const int NON_ASCII_CHAR = 127;

  // check for invalid characters
  size_t url_len = strlen(url);
  for (size_t i = 0; i < url_len; ++i)
  {
    // check if character is a control character or
    // a non-ASCII characters (not handling UTF-8 encoding)
    if (CONTROL_CHAR > url[i] || url[i] > NON_ASCII_CHAR)
    {
      return KC_INVALID;
    }

    // check for reserved characters
    if (strchr(valid_chars, url[i]) == NULL)
    {
      return KC_INVALID;
    }

  }

  // check for leading or trailing slashes
  if (url[0] != '/' || (url[0] == '/' && url[1] == '/'))
  {
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int validate_http_ver(char* http_ver)
{
  // make sure the method exists
  if (http_ver == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // the must be an existing valid method
  if (strcmp(http_ver, KC_HTTP_1) != 0 &&
      strcmp(http_ver, KC_HTTP_2) != 0)
  {
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int validate_http_body(char* body)
{
  // make sure the method exists
  if (body == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  // TODO: validate the Content-Type of the body

  return KC_SUCCESS;

}

//---------------------------------------------------------------------------//
