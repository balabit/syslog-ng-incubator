#ifndef WEBSOCKET_CLIENT_H_INCLUDED
#define WEBSOCKET_CLIENT_H_INCLUDED

#include <libwebsockets.h>

#define CLIENT_RX_BUFFER_BYTES 1024
#define CLIENT_MAX_MESSAGE_QUEUE 1024

int
websocket_client_create(char* protocol, char* address, int port, char* path);

void
websocket_client_disconnect();

void
websocket_client_send_msg(char*);

#endif
