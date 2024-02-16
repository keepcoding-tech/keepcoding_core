#include "hdrs/network/server.h"

#include <stdio.h>

int main()
{
  struct kc_server_t* server = new_server();
  int rez = server->start(server);
  printf("\n rez: %d \n", rez);
  return 0;
}
