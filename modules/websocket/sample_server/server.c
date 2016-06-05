#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "server.h"

static struct a_message ringbuffer[MAX_MESSAGE_QUEUE];
static int ringbuffer_head;

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

static int callback_example( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
  char *msg = (char *) in;
  struct per_session_data *psd = (struct per_session_data *) user;
  int n, m;

	switch( reason )
	{
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
      if (ringbuffer[ringbuffer_head].payload)
        free(ringbuffer[ringbuffer_head].payload);

      ringbuffer[ringbuffer_head].payload = malloc(LWS_PRE + len);
      ringbuffer[ringbuffer_head].len = len;
      memcpy((char *)ringbuffer[ringbuffer_head].payload +
             LWS_PRE, in, len);
      ringbuffer_head = (ringbuffer_head + 1) % MAX_MESSAGE_QUEUE;
      lws_callback_on_writable_all_protocol(lws_get_context(wsi),
                    lws_get_protocol(wsi));
      break;

		default:
      lwsl_notice("Reason %d not handled.\n", reason);
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
		"example-protocol",
		callback_example,
		sizeof(struct per_session_data),
		EXAMPLE_RX_BUFFER_BYTES,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};


int main( int argc, char *argv[] )
{
	struct lws_context_creation_info info;
	memset( &info, 0, sizeof(info) );

	info.port = 8000;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	struct lws_context *context = lws_create_context( &info );

	while( 1 ) {
		lws_service( context, 100 );
  }
	lws_context_destroy( context );

	return 0;
}
