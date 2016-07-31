/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2016 Yilin Li <liyilin1214@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "websocket-source.h"
#include "websocket-transport.h"
#include "ws_api/server.h"
#include <stdio.h>
#include <string.h>
#include "poll-fd-events.h"
#include "logproto/logproto-text-server.h"


void websocket_sd_set_port(LogDriver *source, gint port) {
  WebsocketSrcDriver *self = (WebsocketSrcDriver *)source;
  self->port = port;
}

void
websocket_sd_set_address(LogDriver *source, gchar *address) {
  WebsocketSrcDriver *self = (WebsocketSrcDriver *)source;
  g_free(self->address);
  self->address = g_strdup(address);
}

void
websocket_sd_set_protocol(LogDriver *source, gchar *protocol) {
  WebsocketSrcDriver *self = (WebsocketSrcDriver *)source;
  g_free(self->protocol);
  self->protocol = g_strdup(protocol);
}

void
websocket_sd_set_path(LogDriver *source, gchar *path) {
  WebsocketSrcDriver *self = (WebsocketSrcDriver *)source;
  g_free(self->path);
  self->path = g_strdup(path);
}

void websocket_sd_set_enable_ssl(LogDriver *source, int flag) {
  WebsocketSrcDriver *self = (WebsocketSrcDriver *)source;
  self->enable_ssl = flag;
}

void websocket_sd_set_cert(LogDriver *source, gchar *path) {
  WebsocketSrcDriver *self = (WebsocketSrcDriver *)source;
  g_free(self->cert);
  self->cert = path == NULL ? NULL : g_strdup(path);
}

void websocket_sd_set_key(LogDriver *source, gchar *path) {
  WebsocketSrcDriver *self = (WebsocketSrcDriver *)source;
  g_free(self->key);
  self->key = path == NULL ? NULL : g_strdup(path);
}

void websocket_sd_set_cacert(LogDriver *source, gchar *path) {
  WebsocketSrcDriver *self = (WebsocketSrcDriver *)source;
  g_free(self->cacert);
  self->cacert = path == NULL ? NULL : g_strdup(path);
}

static inline gboolean
start_websocket_server(WebsocketSrcDriver* self)
{
  // TODO: start the server
  // self->client_use_ssl_flag = self->enable_ssl;
  // websocket_server_create(self->protocol, self->port,
  //   self->client_use_ssl_flag, self->cert, self->key, self->cacert);
  return TRUE;
}

static void
create_reader(LogPipe *s)
{
  WebsocketSrcDriver* self = (WebsocketSrcDriver *)s;
  GlobalConfig *cfg = log_pipe_get_config(s);

  LogTransport* transport = log_transport_websocket_new(); // TODO: define the constructor
  PollEvents* poll_events = poll_fd_events_new(transport->fd);
  LogProtoServerOptions* proto_options = &self->reader_options.proto_options.super;
  LogProtoServer* proto = log_proto_text_server_new(transport, proto_options);

  self->reader = log_reader_new(cfg);

  self->reader_options.parse_options.flags |= LP_NOPARSE;

  log_reader_reopen(self->reader, proto, poll_events);
}

static gboolean
websocket_sd_init(LogPipe *s)
{
  WebsocketSrcDriver *self = (WebsocketSrcDriver *) s;
  GlobalConfig* cfg = log_pipe_get_config(&self->super.super.super);

  if (!log_src_driver_init_method(s))
  {
    msg_error("Failed to initialize source driver!", NULL);
    return FALSE;
  }

  log_reader_options_init(&self->reader_options, cfg, "websocket");

  if (!start_websocket_server(self))
    return FALSE;
  create_reader(s);

  log_reader_set_options(self->reader,
                             s,
                             &self->reader_options,
                             STATS_LEVEL1,
                             SCS_WEBSOCKET,
                             self->super.super.id,
                             "websocket");
  log_pipe_append((LogPipe *) self->reader, s);

  if (!log_pipe_init((LogPipe *) self->reader))
  {
    msg_error("Error initializing log_reader", NULL);
    log_pipe_unref((LogPipe *) self->reader);
    return FALSE;
  }

  return TRUE;
}

static void
websocket_socket_deinit(LogReader* reader)
{
  log_pipe_unref((LogPipe *) reader);
  websocket_server_shutdown();
  g_free(reader);
}


static gboolean
websocket_sd_deinit(LogPipe *s)
{
  WebsocketSrcDriver *self = (WebsocketSrcDriver *) s;
  GlobalConfig *cfg = log_pipe_get_config(&self->super.super.super);

  // TODO: close the websocket connection
  if (self->reader)
  {
    // TODO: should we call log_reader_deinit to do this job??
    log_pipe_deinit((LogPipe *) self->reader);
  }

  return log_src_driver_deinit_method(s);
}

static void
websocket_sd_notify(LogPipe *s, gint notify_code, gpointer user_data)
{
  switch (notify_code)
    {
    case NC_CLOSE:
    case NC_READ_ERROR:
      {
        websocket_sd_deinit(s);
        break;
      }
    }
}

static void
websocket_sd_free(LogPipe *source)
{
  WebsocketSrcDriver *self = (WebsocketSrcDriver *) source;
  g_free(self->address);
  g_free(self->path);
  g_free(self->protocol);
  g_free(self->cert);
  g_free(self->key);
  g_free(self->cacert);

  log_reader_free(self->reader); // TODO: make sure it is right
  log_reader_options_destroy(&self->reader_options);
  log_src_driver_free(source);
}

LogDriver *
websocket_sd_new(GlobalConfig *cfg)
{
  WebsocketSrcDriver *self = g_new0(WebsocketSrcDriver, 1);

  log_src_driver_init_instance(&self->super, cfg);

  self->super.super.super.init = websocket_sd_init;
  self->super.super.super.deinit = websocket_sd_deinit;
  self->super.super.super.notify = websocket_sd_notify;
  self->super.super.super.free_fn = websocket_sd_free;

  websocket_sd_set_protocol((LogDriver *) self, "");
  websocket_sd_set_port((LogDriver *) self, 7681);
  websocket_sd_set_address((LogDriver *) self, "127.0.0.1");
  websocket_sd_set_path((LogDriver *) self, "/");
  websocket_sd_set_enable_ssl((LogDriver *) self, FALSE);
  websocket_sd_set_cert((LogDriver *) self, NULL);
  websocket_sd_set_key((LogDriver *) self, NULL);
  websocket_sd_set_cacert((LogDriver *) self, NULL);

  log_reader_options_defaults(&self->reader_options);
  return &self->super.super;
}
