// This file is part of keepcoding_core
// ==================================
//
// http_parser.h
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

/*
 * a network struct
 */

#ifndef KC_HTTP_PARSER_H
#define KC_HTTP_PARSER_H

#include "../datastructs/map.h"

#ifndef KC_HTTP_H
#include "http.h"
#endif

#include <stdio.h>

struct kc_http_request_t;

// ------------------------- PARSE FUNCTIONS --------------------------------//

int http_parse_request_line     (char* request_line, struct kc_http_request_t* req);
int http_parse_request_headers  (char* request_headers, struct kc_http_request_t* req);
int http_parse_request_body     (char* request_body, struct kc_http_request_t* req);

// ------------------------- VALIDATE FUNCTIONS -----------------------------//

int validate_http_method        (char* method);
int validate_http_url           (char* url);
int validate_http_ver           (char* http_ver);
int validate_http_body          (char* body);

//---------------------------------------------------------------------------//

#endif /* KC_HTTP_PARSER_H */
