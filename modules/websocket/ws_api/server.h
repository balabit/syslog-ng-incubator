#ifndef WEBSOCKET_SERVER_H_INCLUDED
#define WEBSOCKET_SERVER_H_INCLUDED
#include <stdio.h>
#define MAX_MESSAGE_QUEUE 512
#define EXAMPLE_RX_BUFFER_BYTES (100)


/**
 * websocket_server_create - create a server listening on specific port
 *
 * @protocol websocket protocol name
 * @port     websocket server listening port
 * @use_ssl  If None-zero value is set, ssl will be used.
 * @cert     (optional) The certificate of server
 * @key      (optional) the private key for the cert
 * @cacert   (optional) A CA cert.
 *
 * Return None-zero if errors occurred.
 */
int
websocket_server_create(char* protocol, int port, int use_ssl, char* cert, char* key, char* cacert);

/**
 * websocket_server_shutdown - shut down websocket server
 */
void
websocket_server_shutdown();

/**
 * websocket_server_broadcast_msg - broadcast message to all client
 *
 * @msg - the message to send
 *
 * Return None-zero if errors occurred.
 */
int
websocket_server_broadcast_msg(char* msg);
#endif
