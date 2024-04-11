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
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//--- MARK: PUBLIC HEADER FUNCTION PROTOTYPES -------------------------------//

struct kc_header_t* new_header     (char* key, char* val);
void                destroy_header (struct kc_header_t* header);

//--- MARK: PUBLIC REQUEST FUNCTION PROTOTYPES ------------------------------//

struct kc_request_t* new_request     (char* method, char* url, char* tcp_vers);
void                 destroy_request (struct kc_request_t* req);
int                  add_req_header  (struct kc_request_t* self, char* key, char* val);

//--- MARK: PUBLIC RESPONSE FUNCTION PROTOTYPES -----------------------------//

struct kc_response_t* new_response     (char* tcp_vers, char* status_code, unsigned char* body);
void                  destroy_response (struct kc_response_t* res);
int                   add_res_header   (struct kc_response_t* self, char* key, char* val);

//--- MARK: PUBLIC SERVER FUNCTION PROTOTYPES -------------------------------//

struct kc_server_t* new_server_IPv4  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server_IPv6  (const char* IP, const unsigned int PORT);
struct kc_server_t* new_server       (const int AF, const char* IP, const unsigned int PORT);
void                destroy_server   (struct kc_server_t* server);
int                 start_server     (struct kc_server_t* self);

int add_get_endpoint     (unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res));
int add_post_endpoint    (unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res));
int add_put_endpoint     (unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res));
int add_delete_endpoint  (unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res));
int add_endpoint         (char* method, unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res));

static void* dispatch_server  (void* socket_fd);

//--- MARK: PRIVATE SERVER FUNCTION PROTOTYPES ------------------------------//

static int _accept_connection  (int server_fd, struct kc_socket_t* socket);

//---------------------------------------------------------------------------//

struct kc_header_t* new_header(char* key, char* val)
{
  // create a new instance to be returned
  struct kc_header_t* new_header = malloc(sizeof(struct kc_header_t));

  // check the allocation of the memory
  if (new_header == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // allocate memory for the key
  new_header->key = malloc(sizeof(char) * strlen(key));
  if (new_header->key == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_header);

    return NULL;
  }

  // allocate memory for the value
  new_header->val = malloc(sizeof(char) * strlen(val));
  if (new_header->val == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_header->key);
    free(new_header);

    return NULL;
  }

  // asign the values
  new_header->key  = key;
  new_header->val  = val;
  new_header->size = strlen(val);

  return new_header;
}

//---------------------------------------------------------------------------//

void destroy_header(struct kc_header_t* header)
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

struct kc_request_t* new_request(char* method, char* url, char* tcp_vers)
{
  struct kc_request_t* new_req = malloc(sizeof(struct kc_request_t));

  // check the allocation of the memory
  if (new_req == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // allocate memory for the method
  new_req->method = malloc(sizeof(char) * strlen(method));
  if (new_req->method == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_req);

    return NULL;
  }

  // allocate memory for the url
  new_req->url = malloc(sizeof(char) * strlen(url));
  if (new_req->url == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_req->method);
    free(new_req);

    return NULL;
  }

  // allocate memory for the tcp_version
  new_req->tcp_vers = malloc(sizeof(char) * strlen(tcp_vers));
  if (new_req->tcp_vers == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_req->method);
    free(new_req->url);
    free(new_req);

    return NULL;
  }

  // asign the values
  new_req->method      = method;
  new_req->url         = url;
  new_req->tcp_vers    = tcp_vers;
  new_req->headers_len = 0;

  // asign the methods
  new_req->add_header = add_req_header;

  return new_req;
}

//---------------------------------------------------------------------------//

void destroy_request(struct kc_request_t* req)
{
  if (req == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(req->method);
  free(req->url);
  free(req->tcp_vers);

  for (int i = 0; i < req->headers_len; ++i)
  {
    destroy_header(req->headers[i]);
  }

  free(req);
}

//---------------------------------------------------------------------------//

int add_req_header(struct kc_request_t* self, char* key, char* val)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // create a new header to be asign
  struct kc_header_t* header = new_header(key, val);
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

struct kc_response_t* new_response(char* tcp_vers, char* status_code, unsigned char* body)
{
  struct kc_response_t* new_res = malloc(sizeof(struct kc_response_t));

  // check the allocation of the memory
  if (new_res == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);
    return NULL;
  }

  // allocate memory for the tcp_vers
  new_res->tcp_vers = malloc(sizeof(char) * strlen(tcp_vers));
  if (new_res->tcp_vers == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_res);

    return NULL;
  }

  // allocate memory for the status_code
  new_res->status_code = malloc(sizeof(char) * strlen(status_code));
  if (new_res->status_code == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_res->tcp_vers);
    free(new_res);

    return NULL;
  }

  // allocate memory for the url
  new_res->body = malloc(sizeof(char) * strlen((char*)body));
  if (new_res->body == NULL)
  {
    log_error(KC_OUT_OF_MEMORY_LOG);

    // first free the memory
    free(new_res->tcp_vers);
    free(new_res->status_code);
    free(new_res);

    return NULL;
  }

  // asign the values
  new_res->tcp_vers    = tcp_vers;
  new_res->status_code = status_code;
  new_res->body        = body;
  new_res->headers_len = 0;

  // asign the methods
  new_res->add_header = add_res_header;

  return new_res;
}

//---------------------------------------------------------------------------//

void destroy_response (struct kc_response_t* res)
{
  if (res == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return;
  }

  free(res->tcp_vers);
  free(res->status_code);
  free(res->body);

  for (int i = 0; i < res->headers_len; ++i)
  {
    destroy_header(res->headers[i]);
  }

  free(res);
}

//---------------------------------------------------------------------------//

int add_res_header(struct kc_response_t* self, char* key, char* val)
{
  if (self == NULL)
  {
    log_error(KC_NULL_REFERENCE_LOG);
    return KC_NULL_REFERENCE;
  }

  // create a new header to be asign
  struct kc_header_t* header = new_header(key, val);
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

//--- MARK: PRIVATE SERVER MEMBERS ------------------------------------------//

struct kc_endpoint_t
{
  unsigned char* endpoint;  // and route endpoint URL

  // the callback function to get called
  int (*callback)  (struct kc_request_t* req, struct kc_response_t* res);
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
  char recv_buffer[1024];

  // receive the message from the connection
  ssize_t recv_ret = recv(*(int*)socket_fd, recv_buffer, 1024, 0);
  if (recv_ret <= KC_SUCCESS)
  {
    return (void*)KC_INVALID;
  }

  // print the message
  recv_buffer[recv_ret] = '\0';
  printf("%s \n", recv_buffer);

  // TODO: generate the request based on the incoming one
  //
  // create the request structure
  struct kc_request_t* req = new_request("GET", "/home", "HTTP/1.1");
  req->add_header(req, "Content-Type", "text/plain");
  req->add_header(req, "Accepts", "*/*");

  char* body =
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
      "<head>\n"
        "<title>keepcoding</title>\n"
      "</head>\n"
      "<body>\n"
        "<h1>Hello, World!</h1>\n"
      "</body>\n"
    "</html>\n";

  struct kc_response_t* res = new_response("HTTP/1.1", "200 OK", (unsigned char*)body);
  res->add_header(res, "Content-Type", "text/html");

  // make the call back
  endpoints[0]->callback(req, res);

  char* response =
    "HTTP/1.1 200 OK\n"
    "Content-Type: text/html\n"
    "\n"
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
      "<head>\n"
        "<title>keepcoding</title>\n"
      "</head>\n"
      "<body>\n"
        "<h1>Hello, World!</h1>\n"
      "</body>\n"
    "</html>\n";

  // send a HTTP response
  send(*(int*)socket_fd, response, strlen(response), 0);

  // close the socket
  close(*(int*)socket_fd);

  // TODO: the dealocation fails, need to be fixed
  // destroy the request
  //destroy_request(req);
  //destroy_response(res);

  return (void*)KC_SUCCESS;
}

//---------------------------------------------------------------------------//

int _accept_connection(int server_fd, struct kc_socket_t* socket)
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

int add_get_endpoint(unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res))
{
  return add_endpoint(KC_GET_METHOD, endpoint, callback);
}

//---------------------------------------------------------------------------//

int add_post_endpoint(unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res))
{
  return add_endpoint(KC_POST_METHOD, endpoint, callback);
}

//---------------------------------------------------------------------------//

int add_put_endpoint(unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res))
{
  return add_endpoint(KC_PUT_METHOD, endpoint, callback);
}

//---------------------------------------------------------------------------//

int add_delete_endpoint(unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res))
{
  return add_endpoint(KC_DELETE_METHOD, endpoint, callback);
}

//---------------------------------------------------------------------------//

int add_endpoint(char* method, unsigned char* endpoint, int (*callback)(struct kc_request_t* req, struct kc_response_t* res))
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
