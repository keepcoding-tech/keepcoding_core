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

#define PORT_WELL_KNOWN_SERVICE 1024
#define PORT_AVAILABLE_FOR_USER 49151
#define PORT_DYNAMIC_OR_PRIVATE 65535

#define IP_INVALID_NETWORK_ADDRESS  0
#define IP_INVALID_FAMILY_ADDRESS  -1

#define KC_IPv4 AF_INET
#define KC_IPv6 AF_INET6

//---------------------------------------------------------------------------//

struct kc_server_t
{
  struct kc_logger_t* _logger; // private member for logging

  int fd;                      // the file descriptor
  struct sockaddr_in* addr;    // socket address

  char* ip;                    // server IP address
  unsigned int port;           // server PORT

  int (*start)   (struct kc_server_t* self);
};

struct kc_server_t* new_server_IPv4  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server_IPv6  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server       (const int AF, const char* IP, const unsigned int PORT);
void                destroy_server   (struct kc_server_t* server);

//---------------------------------------------------------------------------//

void* dispatch  (void* socket_fd);

//---------------------------------------------------------------------------//

#endif /* KC_SERVER_T_H */
