// This file is part of keepcoding_core
// ==================================
//
// socket.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/network/socket.h"
#include "../../hdrs/system/logger.h"
#include "../../hdrs/common.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

struct kc_socket_t* new_socket      (const int AF, const char* ip, const unsigned int port);
void                destroy_socket  (struct kc_socket_t* socket);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static int _validate_socket_ip    (const int AF, const char* ip);
static int _validate_socket_port  (const unsigned int port);

//---------------------------------------------------------------------------//

struct kc_socket_t* new_socket(const int AF, const char* ip, const unsigned int port)
{
  // first of all, validate the IP address
  if (_validate_socket_ip(AF, ip) != KC_SUCCESS)
  {
    return NULL;
  }

  // second of all, validate the PORT number
  if (_validate_socket_port(port) != KC_SUCCESS)
  {
    return NULL;
  }

  // create a new instance to be returned
  struct kc_socket_t* new_socket = malloc(sizeof(struct kc_socket_t));

  // make sure there is momory to alocate
  if (new_socket == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // allocate memory for the socket
  new_socket->addr = malloc(sizeof(struct sockaddr_in));
  if (new_socket->addr == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the socket first
    free(new_socket);

    return NULL;
  }

  // create socket' file descriptor
  new_socket->fd = socket(AF, SOCK_STREAM, 0);

  // assign IP and PORT
  inet_pton(AF, ip, &new_socket->addr->sin_addr.s_addr);
  new_socket->addr->sin_family = AF;
  new_socket->addr->sin_port = htons(port);

  // save the ip and port for easy access
  new_socket->ip = inet_ntoa(new_socket->addr->sin_addr);
  new_socket->port = port;

  return new_socket;
}

//---------------------------------------------------------------------------//

void destroy_socket(struct kc_socket_t* socket)
{
  if (socket == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  if (socket->addr != NULL)
  {
    free(socket->addr);
  }

  free(socket);
}

//---------------------------------------------------------------------------//

static int _validate_socket_ip(int AF, const char* ip)
{
  unsigned char ip_address[sizeof(struct in6_addr)];
  int ret = inet_pton(AF, ip, ip_address);

  // if the specified ip has an
  // invalid format, is invalid
  if (ret == 0)
  {
    log_fatal("Invalid network IP address format.");
    return KC_INVALID;
  }

  // if the specified ip is not
  // IPv4 or IPv6, is invalid
  if (ret == -1)
  {
    log_fatal("Invalid family address: must be IPv4 or IPv6.");
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

static int _validate_socket_port(const unsigned int port)
{
  // if below 0, is invalid
  if (port < 0)
  {
    log_fatal("Port number cannot be negative.");
    return KC_INVALID;
  }

  // if the specified port is below the
  // well known service rage, return warning
  if (port < PORT_WELL_KNOWN_SERVICE)
  {
    log_fatal("Port number below 1024: privileged range.");
    return KC_INVALID;
  }

  // if the specified port between the available
  // and dynamic or private rage, return warning
  if (PORT_AVAILABLE_FOR_USER < port && port < PORT_DYNAMIC_OR_PRIVATE)
  {
    log_fatal("Port number in dynamic/private range (49152-65535).");
    return KC_INVALID;
  }

  // if above 65535, is invalid
  if (port > PORT_DYNAMIC_OR_PRIVATE)
  {
    log_fatal("Port number exceeds maximum allowed (65535).");
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
