#include <stdio.h>
#define MAX_MESSAGE_QUEUE 512
#define EXAMPLE_RX_BUFFER_BYTES (100)

struct per_session_data {
  struct lws *wsi;
  int ringbuffer_tail;
};

struct a_message {
   void *payload;
   size_t len;
};

enum protocols
{
       PROTOCOL_HTTP = 0,
       PROTOCOL_EXAMPLE,
       PROTOCOL_COUNT
};
