#include<unistd.h>
#include "server.h"
#include <stdio.h>


int main() {
  int i, msgqid;
  if (websocket_server_create("example-protocol", 8000, 0, NULL, NULL, NULL, NULL, &msgqid) == -1) {
    printf("server create error\n");
  }
  printf("get msgqid=%d\n", msgqid);
  for (i = 0; i < 10; ++i) {
    usleep(2 * 1000 * 1000);
    websocket_server_broadcast_msg("I'm here", msgqid, 8000);
  }
  websocket_server_shutdown();
  return 0;
}
