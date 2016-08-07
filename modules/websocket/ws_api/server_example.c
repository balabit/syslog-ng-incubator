#include<unistd.h>
#include "server.h"
#include <stdio.h>


int main() {
  int i, msgqid, service_pid;
  if (websocket_server_create("example-protocol", 8000, 0, NULL, NULL, NULL, NULL, &msgqid, &service_pid) != 0) {
    printf("server create error\n");
    return -1;
  }
  for (i = 0; i < 10; ++i) {
    usleep(2 * 1000 * 1000);
    websocket_server_broadcast_msg("I'm server", msgqid, 8000);
  }
  websocket_server_shutdown(service_pid);
  return 0;
}
