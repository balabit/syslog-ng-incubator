

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <libwebsockets.h>
#include <pthread.h>
#include "server.h"
#include "client.h"


static int destroy_flag = 0;
static int connection_flag = 0;

static void INT_HANDLER(int signo) {
  destroy_flag = 1;
}


static int websocket_write_back(struct lws *wsi_in, char *str)
{
	if (str == NULL || wsi_in == NULL)
		return -1;

	int n;
	int len = strlen(str);
	unsigned char *out = NULL;

  // we must prepare the buffer containing the padding.
	out = (unsigned char *)malloc(sizeof(unsigned char)*(LWS_SEND_BUFFER_PRE_PADDING + len + LWS_SEND_BUFFER_POST_PADDING));
	memcpy (out + LWS_SEND_BUFFER_PRE_PADDING, str, len );

	n = lws_write(wsi_in, out + LWS_SEND_BUFFER_PRE_PADDING, len, LWS_WRITE_TEXT);

	lwsl_notice("[websocket_write_back] %s\n", str);

	free(out);

	return n;
}


static int ws_service_callback(
						 struct lws *wsi,
						 enum lws_callback_reasons reason, void *user,
						 void *in, size_t len)
{

	switch (reason) {

		case LWS_CALLBACK_CLIENT_ESTABLISHED:
			lwsl_notice("[Main Service] Connect with server success.\n");
			connection_flag = 1;
			break;

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			lwsl_notice("[Main Service] Connect with server error.\n");
			destroy_flag = 1;
			connection_flag = 0;
			break;

		case LWS_CALLBACK_CLOSED:
			lwsl_notice("[Main Service] LWS_CALLBACK_CLOSED\n");
			destroy_flag = 1;
			connection_flag = 0;
			break;

		case LWS_CALLBACK_CLIENT_RECEIVE:
			lwsl_notice("[Main Service] Client recvived: %s\n", (char *)in);

			break;
		case LWS_CALLBACK_CLIENT_WRITEABLE :
			lwsl_notice("[Main Service] On writeable is called. send some message\n");
			websocket_write_back(wsi, "I'm a client. I'm here.");
			break;

		default:
			break;
	}

	return 0;
}

static void *pthread_routine(void *pdata_in) {
	struct pthread_routine_data *data = pdata_in;

	//* waiting for connection with server done.*/
	while(!connection_flag)
		usleep(1000*20);

	lwsl_notice("[pthread_routine] send a message every 5 seconds\n");
  while (!destroy_flag){
    sleep(5);
    lws_callback_on_writable(data->wsi);
  }
  return NULL;
}

int main(void)
{

  struct sigaction act;
  act.sa_handler = INT_HANDLER;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaction( SIGINT, &act, 0);

	struct lws_context *context = NULL;
	struct lws *wsi = NULL;
	struct lws_protocols protocols[] = {
    {
      "example-protocol",
      &ws_service_callback,
      sizeof(struct per_session_data),
      EXAMPLE_RX_BUFFER_BYTES,
      0,
      NULL,
    },
    {NULL, NULL, 0, 0},
  };

  // create context creation info
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	info.port = CONTEXT_PORT_NO_LISTEN;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	context = lws_create_context(&info);

	if (context == NULL) {
		lwsl_notice("context is NULL.\n");
		return -1;
	}

  // set client connect info
  struct lws_client_connect_info ccinfo;
  memset(&ccinfo, 0, sizeof(ccinfo));

  ccinfo.context = context;
  ccinfo.address = "127.0.0.1";
  ccinfo.path = "/";
  ccinfo.protocol = protocols[0].name;
  ccinfo.port = 8000;
  ccinfo.ssl_connection = 0; // TODO: it should be configurable
  ccinfo.host = "127.0.0.1";
  ccinfo.origin = "127.0.0.1";
  ccinfo.ietf_version_or_minus_one = -1;

  wsi = lws_client_connect_via_info(&ccinfo);
	if (wsi == NULL) {
		lwsl_notice("wsi create error.\n");
		return -1;
	}

	lwsl_notice("wsi create success.\n");

	struct pthread_routine_data pdata;
	pdata.wsi = wsi;
	pdata.context = context;

	pthread_t pid;
	pthread_create(&pid, NULL, pthread_routine, &pdata);
	pthread_detach(pid);

	while(!destroy_flag)
	{
		lws_service(context, 50);
	}

	lws_context_destroy(context);

	return 0;
}
