#include<unistd.h>

#include "server.h"

int main() {
  websocket_server_create("example-protocol", 8000, 0, NULL, NULL, NULL, NULL);
  int i;
  for (i = 0; i < 10; ++i) {
    usleep(2 * 1000 * 1000);
    websocket_server_broadcast_msg("I'm here");
  }
  websocket_server_shutdown();
  return 0;
}
