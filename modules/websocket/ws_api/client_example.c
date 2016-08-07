#include<unistd.h>

#include "client.h"

int main() {
  int i, msgqid, service_pid;
  if (websocket_client_create("example-protocol", "127.0.0.1", 8000, "/", 0, NULL, NULL, NULL, &msgqid, &service_pid) != 0) {
    printf("server create error\n");
    return -1;
  }

  for (i = 0; i < 10; ++i) {
    usleep(2 * 1000 * 1000);
    websocket_client_send_msg("I'm client.", msgqid, 8000);
  }
  websocket_client_disconnect(service_pid);
  return 0;
}
