// This file is part of keepcoding_core
// ==================================
//
// server.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/network/server.h"
#include "../../hdrs/common.h"

#include <stdlib.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

static int start_server  (struct kc_server_t* self);
static int stop_server   (struct kc_server_t* self);

//---------------------------------------------------------------------------//

struct kc_server_t* new_server()
{
  // create a server instance to be returned
  struct kc_server_t* new_server = malloc(sizeof(struct kc_server_t));

  // confirm that there is memory to allocate
  if (new_server == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return NULL;
  }

  new_server->start = start_server;
  new_server->stop  = stop_server;

  return new_server;
}

//---------------------------------------------------------------------------//

void destroy_server(struct kc_server_t* server)
{
  if (server == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(server);
}

//---------------------------------------------------------------------------//

int start_server(struct kc_server_t* self)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // create socket
  const int fd = socket(PF_INET, SOCK_STREAM, 0);

  // bind to open port
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)))
  {
    return KC_LOST_CONNECTION;
  }

  // read port
  socklen_t addr_len = sizeof(addr);
  getsockname(fd , (struct sockaddr*) &addr, &addr_len);
  printf("server started on port: %d\n", (int)ntohs(addr.sin_port));

  // accept incoming connection
  struct sockaddr_storage caddr;
  socklen_t caddr_len = sizeof(caddr) ;
  const int cfd = accept(fd, (struct sockaddr*) &caddr, &caddr_len);

  // read from client with recv!
  char buf[1024];
  recv(cfd, buf, sizeof(buf), 0);

  // print without looking
  printf("client says:\n   %s\n", buf);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int stop_server(struct kc_server_t* self)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
