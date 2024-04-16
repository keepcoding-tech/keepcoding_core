// This file is part of keepcoding_core
// ==================================
//
// server.c
//
// Copyright (c) 2024 Daniel Tanase
// SPDX-License-Identifier: MIT License

#include "../../hdrs/system/logger.h"
#include "../../hdrs/network/server.h"
#include "../../hdrs/common.h"

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

struct kc_server_t* new_server_IPv4  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server_IPv6  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server       (const int AF, const char* IP, const unsigned int PORT);
void                destroy_server   (struct kc_server_t* server);
int                 start_server     (struct kc_server_t* self);

int add_get_endpoint     (unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res));
int add_post_endpoint    (unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res));
int add_put_endpoint     (unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res));
int add_delete_endpoint  (unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res));
int add_endpoint         (char* method, unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res));

static void* dispatch_server  (void* socket_fd);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static int _accept_connection  (int server_fd, struct kc_socket_t* socket);
static int _parse_request      (struct kc_http_request_t** req, char recv_buffer[KC_REQUEST_MAX_SIZE]);

//---------------------------------------------------------------------------//

//--- MARK: PRIVATE MEMBERS -------------------------------------------------//

struct kc_endpoint_t
{
  unsigned char* endpoint;  // and route endpoint URL

  // the callback function to get called
  int (*callback)  (struct kc_http_request_t* req, struct kc_http_response_t* res);
};

// the list of endpoints has to be private
static struct kc_endpoint_t* endpoints[1024];
static unsigned int endpoints_len;

// private member for logging
static struct kc_logger_t* logger;

//---------------------------------------------------------------------------//

struct kc_server_t* new_server_IPv4(const char* IP, const unsigned int PORT)
{
  // initialize the server for IPv4
  return new_server(AF_INET, IP, PORT);
}

//---------------------------------------------------------------------------//

struct kc_server_t* new_server_IPv6(const char* IP, const unsigned int PORT)
{
  // initialize the server for IPv6
  return new_server(AF_INET6, IP, PORT);
}

//---------------------------------------------------------------------------//

struct kc_server_t* new_server(const int AF, const char* IP, const unsigned int PORT)
{
  // create a server instance to be returned
  struct kc_server_t* new_server = malloc(sizeof(struct kc_server_t));
  if (new_server == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // create a new socket
  new_server->socket = new_socket(AF, IP, PORT);
  if (new_server->socket == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the server first
    free(new_server);

    return NULL;
  }

  // create the routes list
  new_server->routes = malloc(sizeof(struct kc_route_t));
  if (new_server->routes == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the server first
    destroy_socket(new_server->socket);
    free(new_server);

    return NULL;
  }

  // asign public member functions for server' routes
  new_server->routes->get    = add_get_endpoint;
  new_server->routes->post   = add_post_endpoint;
  new_server->routes->put    = add_put_endpoint;
  new_server->routes->delete = add_delete_endpoint;

  // allocate memory for the logger
  logger = new_logger(KC_SERVER_LOG_PATH);
  if (logger == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // free the server and socket
    destroy_socket(new_server->socket);
    free(new_server->routes);
    free(new_server);

    return NULL;
  }

  // asign public member functions
  new_server->start = start_server;

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

  for (int i = 0; i < endpoints_len; ++i)
  {
    free(endpoints[i]);
  }

  destroy_logger(logger);
  destroy_socket(server->socket);
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

  // bind the server socket to the IP address
  int ret = bind(self->socket->fd, (struct sockaddr*)self->socket->addr, sizeof(*self->socket->addr));
  if (ret != KC_SUCCESS)
  {
    logger->log(logger, KC_FATAL_LOG,
      KC_NETWORK_ERROR, __FILE__, __LINE__, __func__);
    return KC_NETWORK_ERROR;
  }

  // start listening for connections
  ret = listen(self->socket->fd, KC_SERVER_MAX_CONNECTIONS);
  if (ret != KC_SUCCESS)
  {
    logger->log(logger, KC_FATAL_LOG,
      KC_LOST_CONNECTION, __FILE__, __LINE__, __func__);
    return KC_LOST_CONNECTION;
  }

  printf("\nApplication listening on %s:%d ... \n\n", self->socket->ip, self->socket->port);

  // separate the connections on different threads
  while (1)
  {
    // create a new socket for new connections
    struct kc_socket_t socket;

    // accept the connection for the new socket
    int ret = _accept_connection(self->socket->fd, &socket);
    if (ret != KC_SUCCESS)
    {
      logger->log(logger, KC_FATAL_LOG,
        ret, __FILE__, __LINE__, __func__);
      return ret;
    }

    void* fd = &socket.fd;
    pthread_t id;

    // create a new thread to process the new connection
    pthread_create(&id, NULL, &dispatch_server, fd);
  }

  // first shutdown the connections
  // then return the error, if any
  shutdown(self->socket->fd, SHUT_RDWR);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

void* dispatch_server(void* socket_fd)
{
  char recv_buffer[KC_REQUEST_MAX_SIZE];

  // receive the message from the connection
  ssize_t recv_ret = recv(*(int*)socket_fd, recv_buffer, KC_REQUEST_MAX_SIZE, 0);
  if (recv_ret <= KC_SUCCESS)
  {
    return (void*)KC_INVALID;
  }

  // null terminate the request data
  recv_buffer[recv_ret] = '\0';

  struct kc_http_request_t* req = NULL;
  int ret = _parse_request(&req, recv_buffer);
  if (ret != KC_SUCCESS)
  {
    // TODO: return KC_INTERNAL_SERVER_ERROR to the client
    close(*(int*)socket_fd);

    logger->log(logger, KC_ERROR_LOG, ret, __FILE__, __LINE__, __func__);

    void* return_code = &ret;
    return return_code;
  }

  char* body = "Hello, World!\r\n";

  struct kc_http_response_t* res = new_response("HTTP/1.1", "200 OK", body);
  res->add_header(res, "Content-Type", "text/html");

  // make the call back
  endpoints[0]->callback(req, res);

  char* response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "Hello, World!\r\n";

  // send a HTTP response
  send(*(int*)socket_fd, response, strlen(response), 0);

  // close the socket
  close(*(int*)socket_fd);

  // free the request and response
  destroy_request(req);
  destroy_response(res);

  return (void*)KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int add_get_endpoint(unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  return add_endpoint(KC_GET_METHOD, endpoint, callback);
}

//---------------------------------------------------------------------------//

int add_post_endpoint(unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  return add_endpoint(KC_POST_METHOD, endpoint, callback);
}

//---------------------------------------------------------------------------//

int add_put_endpoint(unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  return add_endpoint(KC_PUT_METHOD, endpoint, callback);
}

//---------------------------------------------------------------------------//

int add_delete_endpoint(unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  return add_endpoint(KC_DELETE_METHOD, endpoint, callback);
}

//---------------------------------------------------------------------------//

int add_endpoint(char* method, unsigned char* endpoint, int (*callback)(struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  // init the endpoint structure
  endpoints[endpoints_len] = malloc(sizeof(struct kc_endpoint_t));
  endpoints[endpoints_len]->endpoint = endpoint;
  endpoints[endpoints_len]->callback = callback;

  // increment the list size
  ++endpoints_len;

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

static int _accept_connection(int server_fd, struct kc_socket_t* socket)
{
  // create a new socket for the client
  struct sockaddr_in client_addr;
  int client_addr_size = sizeof(struct sockaddr_in);

  // accept the connection
  int client_fd = accept(server_fd, (struct sockaddr*)&client_addr,
    (socklen_t * restrict)&client_addr_size);

  // serialize the data
  socket->addr = &client_addr;
  socket->fd = client_fd;

  // make sure the connection was made succesfully
  if (client_fd <= 0)
  {
    return KC_INVALID;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

static int _parse_request(struct kc_http_request_t** req, char recv_buffer[KC_REQUEST_MAX_SIZE])
{
  // mark the end of the header section and the body section
  int buffer_len = strlen(recv_buffer);
  for (int i = 0; i < buffer_len - 2; ++i)
  {
    if (recv_buffer[i] == '\n' && recv_buffer[i + 1] == '\n')
    {
      recv_buffer[i + 1] = '|';
    }
  }

  // separate the request data
  char* request  = strtok(recv_buffer, "\r\n");
  char* headers  = strtok(NULL, "|");
  //char* body     = strtok(NULL, "|");

  if (request == NULL || headers == NULL)
  {
    return KC_FORMAT_ERROR;
  }

  // parse the request line
  char* tmp_method   = strtok(request, " ");
  char* tmp_url      = strtok(NULL, " ");
  char* tmp_http_ver = strtok(NULL, "\r\n");

  if (tmp_method == NULL || tmp_url == NULL || tmp_http_ver == NULL)
  {
    return KC_FORMAT_ERROR;
  }

  // create the request structure
  (*req) = new_request(tmp_method, tmp_url, tmp_http_ver);
  if ((*req) == NULL)
  {
    return KC_OUT_OF_MEMORY;
  }

  // parse the headers
  char* tmp_header = strtok(headers, "\r\n");
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
      (*req)->add_header((*req), key, val);
    }

    tmp_header = strtok(NULL, "\r\n");
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//
