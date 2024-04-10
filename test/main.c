#include "../hdrs/system/logger.h"
#include "../hdrs/common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>
#include <microhttpd.h>

#define PORT 8888

size_t write_chunk(void* data, size_t size, size_t nmemb, void* userdata);

static int answer_to_connection(void *cls, struct MHD_Connection *connection,
    const char *url, const char *method, const char *version,
    const char *upload_data, size_t *upload_data_size, void **con_cls);

int print_out_key (void *cls, enum MHD_ValueKind kind, const char *key, const char *value);

struct kc_res_t
{
  char* str;
  size_t len;
};

void test_daemon(void)
{
  struct MHD_Daemon *daemon;

  daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                            &answer_to_connection, NULL, MHD_OPTION_END);
  if (NULL == daemon)
  {
    printf("Failed to start server\n");
    return;
  }

  printf("Server running on port %d\n", PORT);

  getchar();

  MHD_stop_daemon(daemon);
}

void test_curl(void)
{
  CURL* curl = curl_easy_init();

  if (curl == NULL)
  {
    log_error("Failed to init CURL");
    return;
  }

  // init response
  struct kc_res_t response;
  response.str = malloc(1);
  response.len = 0;

  char url[2048];
  char par1[] = "a string with space";
  char par2[] = "special characters %$*";

  char* par1_safe = curl_easy_escape(curl, par1, 0);
  char* par2_safe = curl_easy_escape(curl, par2, 0);

  sprintf(url, "https://improved-lamp-v5659v99g9pcxrj5-8888.app.github.dev/home?key1=%s&key2=%s", par1_safe, par2_safe);

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_chunk);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK)
  {
    log_error(curl_easy_strerror(res));
    return;
  }

  printf("%s", response.str);

  curl_free(par1_safe);
  curl_free(par2_safe);
  curl_easy_cleanup(curl);

  free(response.str);
}

int main(int argc, char **argv)
{
  if (argv[1] != NULL)
  {
    if (strcmp(argv[1], "daemon") == 0)
    {
      test_daemon();
    }

    if (strcmp(argv[1], "curl") == 0)
    {
      test_curl();
    }
  }

  return 0;
}

static int answer_to_connection(void *cls, struct MHD_Connection *connection,
    const char *url, const char *method, const char *version,
    const char *upload_data, size_t *upload_data_size, void **con_cls)
{
  const char *page = "<html><body>Hello, world!</body></html>";
  struct MHD_Response *response;

  response = MHD_create_response_from_buffer(strlen(page), (void *)page,
    MHD_RESPMEM_PERSISTENT);

  MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);

  printf ("%s %s %s\n", method, url, version);
  MHD_get_connection_values(connection, MHD_HEADER_KIND, &print_out_key, NULL);

  return MHD_NO;
}

int print_out_key (void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
  printf ("%s: %s\n", key, value);
  return MHD_YES;
}

size_t write_chunk(void* data, size_t size, size_t nmemb, void* userdata)
{
  size_t real_size = size * nmemb;
  struct kc_res_t* response = (struct kc_res_t*)userdata;


  char* ptr = realloc(response->str, response->len + real_size + 1);
  if (ptr == NULL)
  {
    log_error("Out of memory");
    return 0;
  }

  response->str = ptr;
  memcpy(&(response->str[response->len]), data, real_size);
  response->len += real_size;
  response->str[response->len] = '\0';

  return real_size;
}

