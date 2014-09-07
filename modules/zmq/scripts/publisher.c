#include <stdio.h>
#include <zmq.h>
#include "zhelpers.h"


int main(int argc, char* argv[])
{
    void *context = zmq_ctx_new();
    void *publisher = zmq_socket(context, ZMQ_PUB);

    int i = 0;
    char nmessage[1024];
    char *message;
    const char *mb = "Message number ";

    zmq_bind(publisher, "tcp://*:5556");

    while(1)
    {
        sprintf(nmessage, "%d", i);
        message = strcpy(message, mb);
        strcat(message, nmessage);


        s_send(publisher, message);
        printf("\"%s\" sent!\n", message);

        i++;
        sleep(1);
    }

    zmq_close(publisher);
    zmq_ctx_destroy(context);
    return 0;
}
