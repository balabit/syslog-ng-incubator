#include <time.h>
#include <stdio.h>
#include <zmq.h>
#include "zhelpers.h"

double
get_number_of_message_per_seconds(unsigned int start_time, unsigned int end_time, int number_of_messages)
{
  if (end_time-start_time == 0)
      return number_of_messages;
  return number_of_messages/(end_time-start_time);
}


int main(int argc, char* argv[])
{
  int number_of_messages = 1;
  char buffer[200] = {0};
  char location[22];
  unsigned int start_time = (unsigned)time(NULL);
  unsigned int end_time;

  if (argc == 3)
  {
    strcat(location, "tcp://localhost:");
    strcat(location, argv[1]);
    number_of_messages = atoi(argv[2]);
  }
  else
  {
    printf("Example usage: ./push 44444 1000");
    return;
  }

  printf("Send %d messages\n", number_of_messages);

  void *context = zmq_ctx_new();
  void *sink = zmq_socket(context, ZMQ_PUSH);

  zmq_connect (sink, location);
  int i = 0;
  int rc = 0;
  for (i = 0; i < number_of_messages; i++)
  {
    rc = sprintf(buffer, "Polipokelore! %d\n", i);
    rc = zmq_send(sink, buffer, rc, 0);
  }
  end_time = (unsigned)time(NULL);
  printf("%f msg/sec\n", get_number_of_message_per_seconds(start_time, end_time, number_of_messages));

  zmq_close(sink);
  zmq_ctx_destroy(context);
  return 0;
}
