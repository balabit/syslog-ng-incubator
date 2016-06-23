#include<unistd.h>

#include "client.h"

int main() {
  websocket_client_create("example-protocol", "127.0.0.1", 8000, "/", 0, NULL, NULL, NULL);

  int i;
  for (i = 0; i < 10; ++i) {
    usleep(2 * 1000 * 1000);
    websocket_client_send_msg("I'm here");
  }
  websocket_client_disconnect();
  return 0;
}
