#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include "zhelpers.h"


int main(int argc, char **argv)
{
    char *port = "5556";

    if (argc==2)
    {
        port = argv[1];
    }
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_PULL);

    char *connection_base = "tcp://localhost:";
    char *connection_string = malloc(strlen(connection_base) * sizeof(char) + strlen(port) * sizeof(char));
    strcpy(connection_string, connection_base);
    strcat(connection_string, port);

    printf("connection string: %s\n", connection_string);
    if (zmq_connect(subscriber, connection_string) == 0)
    {
        printf("Successfuly connected to port %s!\n", port);
        free(connection_string);
    }
    else
    {
        printf("Something went wrong, failed to connect to port %s. Error code: %d\n", port, errno);
        free(connection_string);
        zmq_close(subscriber);
        zmq_ctx_destroy(context);
        return 1;
    }

    char *receive_everything = "";

    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, receive_everything, strlen(receive_everything));
    char *string;
    printf("Waiting for receive message!\n");
    while(1)
    {
        string = s_recv (subscriber);
        printf("%s\n", string);
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}
