#ifndef WEBSOCKET_CLIENT_H_INCLUDED
#define WEBSOCKET_CLIENT_H_INCLUDED

#include <libwebsockets.h>

#define CLIENT_RX_BUFFER_BYTES 1024
#define CLIENT_MAX_MESSAGE_QUEUE 1024

/**
 * websocket_client_create - create a client connection to communicate with the server
 *
 * @protocol websocket protocol
 * @address  websocket server listening address
 * @port     websocket server listening port
 * @path     the path of the websocket server service
 * @use_ssl  If None-zero value is set, ssl will be used. If you want to allow
 *           self-signed certificate, please use 2. Otherwise you can use 1.
 * @cert     (optional) If the server wants us to present a valid SSL client
 *           certificate, you can the filepath of it
 * @key      (optional) the private key for the cert
 * @cacert   (optional) A CA cert and CRL can be used to validate the cert send
 *           by the server
 *
 * Return None-zero if errors occurred.
 */
int
websocket_client_create(char* protocol, char* address, int port, char* path, int use_ssl, char* cert, char* key, char* cacert);

/**
 * websocket_client_disconnect - disconnect from the websocket server
 */
void
websocket_client_disconnect();

/**
 * websocket_client_send_msg - send message from the client to the server
 *
 * @msg - the message to send
 *
 * Return None-zero if errors occurred.
 */
int
websocket_client_send_msg(char* msg);
#endif
