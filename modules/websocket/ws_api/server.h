#ifndef WEBSOCKET_SERVER_H_INCLUDED
#define WEBSOCKET_SERVER_H_INCLUDED
#include <stdio.h>
#define MAX_MESSAGE_QUEUE 512
#define EXAMPLE_RX_BUFFER_BYTES (100)

#define QUEUE_DATA_LEN 1024

/**
 * for message queue
 */
struct msg_buf
{
    long mtype;
    char data[QUEUE_DATA_LEN];
};

/**
 * websocket_server_create - create a server listening on specific port
 *
 * @protocol websocket protocol name
 * @port     websocket server listening port
 * @use_ssl  If None-zero value is set, ssl will be used.
 * @cert     (optional) The certificate of server
 * @key      (optional) the private key for the cert
 * @cacert   (optional) A CA cert.
 * @fd       (optional) If fd is given and not NULL, all data server received
 *                      will be sent to the fd.
 * @msgqid   msgqid the message queue id, you are sending messages to this queue latter
 * @service_pid         The pid of the service process will be stored in the service_pid address
 *
 * Return None-zero if errors occurred.
 */
int
websocket_server_create(char* protocol, int port, int use_ssl, char* cert, char* key, char* cacert, int* fd, int* msgqid, int* service_pid);

/**
 * websocket_server_shutdown - shut down websocket server
 *
 * NOTICE: This function must be called in the main process which called
 * websocket_server_create before
 */
void
websocket_server_shutdown(int service_pid);

/**
 * websocket_server_broadcast_msg - broadcast message to all client
 *
 * @msg       the message to send
 * @msgqid    msgqid the message queue id, you are sending message to this queue
 * @port      which port you want to broadcast the message, the port is
 *            needed because we need it to determine the type of the message
 * @service_pid the pid of the service process
 *
 * Return None-zero if errors occurred.
 */
int
websocket_server_broadcast_msg(char* msgj, int msgqid, int port);
#endif
