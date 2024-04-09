#include "../hdrs/system/logger.h"
#include "../hdrs/common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <curl/curl.h>

size_t write_chunk(void* data, size_t size, size_t nmemb, void* userdata);

struct kc_res_t
{
  char* str;
  size_t len;
};

int main(void)
{
  CURL* curl = curl_easy_init();

  if (curl == NULL)
  {
    log_error("Failed to init CURL");
    return 1;
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

  printf("%s\n", par1_safe);
  printf("%s\n", par2_safe);

  sprintf(url, "0.0.0.0:8000/home?key1=%s&key2=%s", par1_safe, par2_safe);

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_chunk);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK)
  {
    log_error(curl_easy_strerror(res));
    return 1;
  }

  printf("%s", response.str);

  curl_free(par1_safe);
  curl_free(par2_safe);
  curl_easy_cleanup(curl);

  free(response.str);

  return 0;
}

size_t write_chunk(void* data, size_t size, size_t nmemb, void* userdata)
{
  size_t real_size = size * nmemb;
  struct kc_res_t* response = (struct kc_res_t*)userdata;


  char* ptr = realloc(response->str, response->len + real_size + 1);
  if (ptr == NULL)
  {
    log_error("Out of memory");
    return CURL_WRITEFUNC_ERROR;
  }

  response->str = ptr;
  memcpy(&(response->str[response->len]), data, real_size);
  response->len += real_size;
  response->str[response->len] = '\0';

  return real_size;
}

