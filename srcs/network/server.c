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

//--- MARK: PUBLIC ENDPOINT FUNCTION PROTOTYPES -----------------------------//

//---------------------------------------------------------------------------//

struct kc_endpoint_t* new_endpoint(char* method, char* url)
{
  // create a new instance to be returned
  struct kc_endpoint_t* new_endpoint = malloc(sizeof(struct kc_endpoint_t));

  // check the allocation of memory
  if (new_endpoint == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  new_endpoint->method = (char*)malloc(sizeof(char) * strlen(method) + 1);
  if (new_endpoint->method == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_endpoint);

    return NULL;
  }

  new_endpoint->url = (char*)malloc(sizeof(char) * strlen(url) + 1);
  if (new_endpoint->url == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_endpoint->method);

    return NULL;
  }

  // asign the values
  strcpy(new_endpoint->method, method);
  strcpy(new_endpoint->url, url);

  return new_endpoint;
}

//---------------------------------------------------------------------------//

void destroy_endpoint  (struct kc_endpoint_t* endpoint)
{
  if (endpoint == NULL)
  {
    return;
  }

  if (endpoint->method != NULL)
  {
    free(endpoint->method);
  }

  if (endpoint->url != NULL)
  {
    free(endpoint->url);
  }

  free(endpoint);
}

//--- MARK: PUBLIC FUNCTION PROTOTYPES --------------------------------------//

struct kc_server_t* new_server_IPv4  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server_IPv6  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server       (const int AF, const char* IP, const unsigned int PORT);
void                destroy_server   (struct kc_server_t* server);
int                 start_server     (struct kc_server_t* self);
int                 send_msg_server  (int client_fd, struct kc_http_response_t* res);

static void* dispatch  (void* dispatch_information);

//--- MARK: PRIVATE FUNCTION PROTOTYPES -------------------------------------//

static int _accept_connection      (int server_fd, struct kc_socket_t* socket);
static void _add_options_endpoint  (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
static void _add_get_endpoint      (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
static void _add_head_endpoint     (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
static void _add_post_endpoint     (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
static void _add_put_endpoint      (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
static void _add_delete_endpoint   (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
static void _add_trace_endpoint    (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
static void _add_connect_endpoint  (char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
static void _add_endpoint          (char* method, char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res));
static int _parse_request          (struct kc_http_request_t* req, char recv_buffer[KC_REQUEST_MAX_SIZE]);

//---------------------------------------------------------------------------//

//--- MARK: PRIVATE MEMBERS -------------------------------------------------//

struct kc_dispatch_info_t
{
  struct kc_server_t* server;
  int client_fd;
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

  // asign public member functions for server' routes
  new_server->routes->options = _add_options_endpoint;
  new_server->routes->get     = _add_get_endpoint;
  new_server->routes->head    = _add_head_endpoint;
  new_server->routes->post    = _add_post_endpoint;
  new_server->routes->put     = _add_put_endpoint;
  new_server->routes->delete  = _add_delete_endpoint;
  new_server->routes->trace   = _add_trace_endpoint;
  new_server->routes->connect = _add_connect_endpoint;

  // asign public member functions
  new_server->start = start_server;
  new_server->send  = send_msg_server;

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

    struct kc_dispatch_info_t* info = malloc(sizeof(struct kc_dispatch_info_t));
    info->server = self;
    info->client_fd = socket.fd;

    pthread_t id;

    // create a new thread to process the new connection
    pthread_create(&id, NULL, &dispatch, (void*)info);
  }

  // first shutdown the connections
  // then return the error, if any
  shutdown(self->socket->fd, SHUT_RDWR);

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int send_msg_server(int client_fd, struct kc_http_response_t* res)
{
  // the file descriptor must be greater then 1
  if (client_fd <= 0)
  {
    return KC_INVALID_ARGUMENT;
  }

  // check if the response was generated
  if (res == NULL)
  {
    return KC_NULL_REFERENCE;
  }

  char* response = (char*)malloc(sizeof(char) * KC_RESPONSE_MAX_SIZE);

  sprintf(response, "%s %s\r\n", res->http_ver, res->status_code);
  for (int i = 0; i < res->headers_len; ++i)
  {
    char* header = (char*)malloc(sizeof(char) * KC_MAX_HEADER_LENGTH);
    sprintf(header, "%s: %s\r\n", res->headers[i]->key, res->headers[i]->val);
    strcat(response, header);
  }

  strcat(response, "\r\n");
  strcat(response, res->body);
  strcat(response, "\r\n");

  // send a HTTP response
  send(client_fd, response, strlen(response), 0);

  return KC_SERVER_SEND_MSG;
}

//---------------------------------------------------------------------------//

void* dispatch(void* dispatch_information)
{
  char recv_buffer[KC_REQUEST_MAX_SIZE];
  struct kc_dispatch_info_t* dispatch_info = (struct kc_dispatch_info_t*)dispatch_information;

  // receive the message from the connection
  ssize_t recv_ret = recv(dispatch_info->client_fd, recv_buffer, KC_REQUEST_MAX_SIZE, 0);
  if (recv_ret <= KC_SUCCESS)
  {
    close(dispatch_info->client_fd);
    pthread_exit((void*)KC_INVALID);
  }

  // null terminate the request data
  recv_buffer[recv_ret] = '\0';

  // parse the request buffer and generate the 
  // a new request structure to be used later
  struct kc_http_request_t* req = new_request();
  int ret = _parse_request(req, recv_buffer);
  if (ret != KC_SUCCESS)
  {
    log_error(kc_error_msg[ret + 1]);
    close(dispatch_info->client_fd);
    pthread_exit((void*)KC_INVALID);
  }

  // set the file descriptor of the client
  req->set_client_fd(req, dispatch_info->client_fd);

  // create the response structure with all 
  // the general headers to be used later
  struct kc_http_response_t* res = new_response();
  res->set_http_ver(res, KC_HTTP_1);
  res->set_status_code(res, KC_HTTP_STATUS_200);

  // TODO: add general headers
  res->add_header(res, "Content-Type", "text/plain");

  // search the callback
  struct kc_endpoint_t* endpoint = NULL;
  for (int i = 0; i < endpoints_len; ++i)
  {
    if (strcmp(endpoints[i]->url, req->url) == 0)
    {
      // save the endpoint
      endpoint = endpoints[i];
      break;
    }
  }

  // page not found, return 404
  if (endpoint == NULL)
  {
    res->set_status_code(res, KC_HTTP_STATUS_404);
    res->add_header(res, "Content-Type", "text/html");
    res->set_body(res, "<h1>404 Page Not Found</h1>\r\n");
    send_msg_server(dispatch_info->client_fd, res);
  }
  else if (strcmp(endpoint->method, req->method) != 0)
  {
    res->set_status_code(res, KC_HTTP_STATUS_400);
    res->add_header(res, "Content-Type", "text/html");
    res->set_body(res, "<h1>400 Bad Request</h1>\r\n");
    send_msg_server(dispatch_info->client_fd, res);
  }
  else
  {
    endpoint->callback(dispatch_info->server, req, res);
  }


  // close the socket
  close(dispatch_info->client_fd);

  // free the request and response
  destroy_request(req);
  destroy_response(res);

  pthread_exit((void*)KC_SUCCESS);
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

static void _add_options_endpoint(char* endpoint, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  _add_endpoint(KC_HTTP_METHOD_OPTIONS, endpoint, callback);
}

//---------------------------------------------------------------------------//

static void _add_get_endpoint(char* endpoint, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  _add_endpoint(KC_HTTP_METHOD_GET, endpoint, callback);
}

//---------------------------------------------------------------------------//

static void _add_head_endpoint(char* endpoint, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  _add_endpoint(KC_HTTP_METHOD_HEAD, endpoint, callback);
}

//---------------------------------------------------------------------------//

static void _add_post_endpoint(char* endpoint, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  _add_endpoint(KC_HTTP_METHOD_POST, endpoint, callback);
}

//---------------------------------------------------------------------------//

static void _add_put_endpoint(char* endpoint, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  _add_endpoint(KC_HTTP_METHOD_PUT, endpoint, callback);
}

//---------------------------------------------------------------------------//

static void _add_delete_endpoint(char* endpoint, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  _add_endpoint(KC_HTTP_METHOD_DELETE, endpoint, callback);
}

//---------------------------------------------------------------------------//

static void _add_trace_endpoint(char* endpoint, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  _add_endpoint(KC_HTTP_METHOD_TRACE, endpoint, callback);
}

//---------------------------------------------------------------------------//

static void _add_connect_endpoint(char* endpoint, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  _add_endpoint(KC_HTTP_METHOD_CONNECT, endpoint, callback);
}

//---------------------------------------------------------------------------//

static void _add_endpoint(char* method, char* url, int (*callback)(struct kc_server_t* self, struct kc_http_request_t* req, struct kc_http_response_t* res))
{
  if (url == NULL || callback == NULL)
  {
    log_fatal(KC_INVALID_ARGUMENT_LOG);
    return;
  }

  // create a new endpoint
  endpoints[endpoints_len] = new_endpoint(method, url);

  // init the endpoint' callback function
  endpoints[endpoints_len]->callback = callback;

  // increment the list size
  ++endpoints_len;
}

//---------------------------------------------------------------------------//

static int _parse_request(struct kc_http_request_t* req, char recv_buffer[KC_REQUEST_MAX_SIZE])
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
  char* request_line     = strtok(recv_buffer, "\r\n");
  char* request_headers  = strtok(NULL, "|");
  char* request_body     = strtok(NULL, "|");

  int ret = KC_SUCCESS;

  // parse the request line
  ret = http_parse_request_line(request_line, req);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  // parse the headers
  ret = http_parse_request_headers(request_headers, req);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  // parse the body only if the method has a body
  if (strcmp(req->method, KC_HTTP_METHOD_GET) == 0)
  {
    return KC_SUCCESS;
  }

  // TODO: get the Content-Type from the headers and pass it to the parse fun
  ret = http_parse_request_body(request_body, req);
  if (ret != KC_SUCCESS)
  {
    return ret;
  }

  return KC_SUCCESS;
}

//---------------------------------------------------------------------------//

