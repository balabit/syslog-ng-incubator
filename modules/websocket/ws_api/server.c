#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "server.h"


struct per_session_data {
  struct lws *wsi;
  int ringbuffer_tail;
};

struct a_message {
   void *payload;
   size_t len;
};

static struct a_message ringbuffer[MAX_MESSAGE_QUEUE];
static int ringbuffer_head;
static int destroy_flag = 0;
static int listening=0;
static struct lws_context *context = NULL;
static pthread_t service_pid;
static int FD[2]; // use pipeline to provide a file descriptor
static int use_fd;

static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
  switch( reason )
  {
    case LWS_CALLBACK_HTTP:
      lws_serve_http_file( wsi, "index.html", "text/html", NULL, 0 );
      break;
    default:
      break;
  }

  return 0;
}

struct payload
{
  unsigned char data[LWS_SEND_BUFFER_PRE_PADDING + EXAMPLE_RX_BUFFER_BYTES + LWS_SEND_BUFFER_POST_PADDING];
  size_t len;
} received_payload;


static int ws_server_callback( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
  struct per_session_data *psd = (struct per_session_data *) user;
  int n, m;

  switch( reason )
  {
    case LWS_CALLBACK_PROTOCOL_INIT:
      listening = 1;
      break;
    case LWS_CALLBACK_ESTABLISHED:
      lwsl_info("%s: LWS_CALLBACK_ESTABLISHED\n", __func__);
      psd->ringbuffer_tail = ringbuffer_head;
      psd->wsi = wsi;
      break;

    case LWS_CALLBACK_PROTOCOL_DESTROY:
      lwsl_notice("%s: Protocol cleaning up\n", __func__);
      for (n = 0; n < sizeof ringbuffer / sizeof ringbuffer[0]; n++)
        if (ringbuffer[n].payload)
          free(ringbuffer[n].payload);
      break;

    case LWS_CALLBACK_SERVER_WRITEABLE:
      while (psd->ringbuffer_tail != ringbuffer_head) {
        m = ringbuffer[psd->ringbuffer_tail].len;
        n = lws_write(wsi, (unsigned char *)
             ringbuffer[psd->ringbuffer_tail].payload +
             LWS_PRE, m, LWS_WRITE_TEXT);
        if (n < 0) {
          lwsl_err("ERROR %d writing to mirror socket\n", n);
          return -1;
        }
        if (n < m)
          lwsl_err("Write %d vs %d\n", n, m);
        psd->ringbuffer_tail = (psd->ringbuffer_tail + 1) % MAX_MESSAGE_QUEUE;
      }
      break;

    case LWS_CALLBACK_RECEIVE:
      lwsl_notice("Message recieved from the client: %s\n", in);
      if (use_fd) { //Writing the message to the pipline
         write(FD[1], in, len + 1);
      }
      break;

    default:
      lwsl_debug("Reason %d not handled.\n", reason);
      break;
  }

  return 0;
}


static struct lws_protocols protocols[] =
{
  /* The first protocol must always be the HTTP handler */
  {
    "http-only",   /* name */
    callback_http, /* callback */
    0,             /* No per session data. */
    0,             /* max frame size / rx buffer */
  },
  {
    NULL,
    ws_server_callback,
    sizeof(struct per_session_data),
    EXAMPLE_RX_BUFFER_BYTES,
  },
  { NULL, NULL, 0, 0 } /* terminator */
};

static void *
pthread_routine(void *pdata_in) {

  while(!destroy_flag)
  {
    lws_service( context, 50 );
  }
  lwsl_notice("[pthread_routine] Service has ended.\n");
  return NULL;
}


int
websocket_server_create(char* protocol, int port, int use_ssl, char* cert, char* key, char* cacert, int* fd) {
  struct lws_context_creation_info info;
  memset( &info, 0, sizeof(info) );

  protocols[1].name = protocol;
  info.port = port;
  info.protocols = protocols;
  info.gid = -1;
  info.uid = -1;
  info.ssl_cert_filepath = NULL;
  info.ssl_private_key_filepath = NULL;
  info.ssl_ca_filepath = NULL;

  if (use_ssl) {
    info.ssl_cert_filepath = cert;
    info.ssl_private_key_filepath = key;
    info.ssl_ca_filepath = cacert;
    info.options |= LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;
  }

  context = lws_create_context( &info );

  use_fd = (fd != NULL);
  if (use_fd) {
    pipe(FD);
    *fd = FD[0]; // return the file descriptor for reading
  }

  pthread_create(&service_pid, NULL, pthread_routine, NULL);

  return 0;
}


int
websocket_server_broadcast_msg(char* msg) {
  while(!listening && !destroy_flag)
    usleep(1000*20);
  if (!listening && destroy_flag) {
    lwsl_notice("The server has been shutdown. You can't broadcast messages\n");
    return 1;  // If we are not connected. We cant send the message
  }
  lwsl_notice("broadcasting message: %s.\n", msg);
  int len=strlen(msg);
  if (ringbuffer[ringbuffer_head].payload)
    free(ringbuffer[ringbuffer_head].payload);

  ringbuffer[ringbuffer_head].payload = malloc(LWS_PRE + len);
  ringbuffer[ringbuffer_head].len = len;
  memcpy((char *)ringbuffer[ringbuffer_head].payload +
         LWS_PRE, msg, len);
  ringbuffer_head = (ringbuffer_head + 1) % MAX_MESSAGE_QUEUE;
  lws_callback_on_writable_all_protocol(context, &protocols[1]);
  return 0;
}


void
websocket_server_shutdown() {
  destroy_flag = 1;
  pthread_join(service_pid, NULL);
  lws_context_destroy( context );
}
