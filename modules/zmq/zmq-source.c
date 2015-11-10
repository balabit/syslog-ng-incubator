/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Laszlo Meszaros <lacienator@gmail.com>
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

#include <zmq.h>

#include "zmq-module.h"
#include "zmq-transport.h"

#include "poll-fd-events.h"
#include "logproto/logproto-text-server.h"

void zmq_sd_set_address(LogDriver *source, const gchar *address);
void zmq_sd_set_port(LogDriver *source, gint port);

typedef struct _ZMQReaderContext
{
  LogReader* reader;
  void* context;
} ZMQReaderContext;

void
zmq_sd_set_address(LogDriver *source, const gchar *address)
{
    ZMQSourceDriver *self = (ZMQSourceDriver *)source;
    g_free(self->address);
    self->address = g_strdup(address);
}

void
zmq_sd_set_port(LogDriver *source, gint port)
{
    ZMQSourceDriver *self = (ZMQSourceDriver *)source;
    self->port = port;
    msg_info("Port set successfully!", evt_tag_int("Port: ", self->port), NULL);
}

static void
zmq_sd_create_reader(LogPipe *s)
{
  ZMQSourceDriver* self = (ZMQSourceDriver *)s;
  GlobalConfig *cfg = log_pipe_get_config(s);

  LogTransport* transport = log_transport_zmq_new(self->socket);
  PollEvents* poll_events = poll_fd_events_new(transport->fd);
  LogProtoServerOptions* proto_options = &self->reader_options.proto_options.super;
  LogProtoServer* proto = log_proto_text_server_new(transport, proto_options);

  self->reader = log_reader_new(cfg);

  self->reader_options.parse_options.flags |= LP_NOPARSE;

  log_reader_reopen(self->reader, proto, poll_events);
}

gchar *
zmq_sd_get_address(ZMQSourceDriver* self)
{
  return g_strdup_printf("tcp://%s:%d", self->address, self->port);
}

gboolean
zmq_sd_create_context(ZMQSourceDriver* self)
{
  errno = 0;
  self->context = zmq_ctx_new();
  self->socket = zmq_socket(self->context, ZMQ_PULL);

  if (errno != 0)
  {
    msg_error("Something went wrong during create socket!", evt_tag_errno("errno", errno), NULL);
    return FALSE;
  }

  gchar* address = zmq_sd_get_address(self);

  if (zmq_connect(self->socket, address) != 0)
  {
    msg_error("Failed to connect!", evt_tag_str("Connect address", address), evt_tag_errno("Error", errno),NULL);
    g_free(address);
    return FALSE;
  }

  g_free(address);
  return TRUE;
}

gchar*
zmq_sd_get_persist_name(ZMQSourceDriver* self)
{
    return g_strdup_printf("zmq_source:%s:%d", self->address, self->port);
}

static gboolean
zmq_sd_init(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;
  GlobalConfig* cfg = log_pipe_get_config(&self->super.super.super);

  if (!log_src_driver_init_method(s))
  {
    msg_error("Failed to initialize source driver!", NULL);
    return FALSE;
  }

  gchar* persist_name = zmq_sd_get_persist_name(self);

  ZMQReaderContext* reader_context = cfg_persist_config_fetch(cfg, persist_name);

  g_free(persist_name);

  log_reader_options_init(&self->reader_options, cfg, "zmq");

  if (reader_context)
  {
    self->context = reader_context->context;
    self->reader = reader_context->reader;
    g_free(reader_context);
  }
  else
  {
    if (!zmq_sd_create_context(self))
      return FALSE;
    zmq_sd_create_reader(s);
  }

  log_reader_set_options(self->reader,
                             s,
                             &self->reader_options,
                             STATS_LEVEL1,
                             SCS_ZMQ,
                             self->super.super.id,
                             "zmq");
  log_pipe_append((LogPipe *) self->reader, s);

  if (!log_pipe_init((LogPipe *) self->reader))
  {
    msg_error("Error initializing log_reader", NULL);
    log_pipe_unref((LogPipe *) self->reader);
    self->reader = NULL;

    return FALSE;
  }

  return TRUE;
}

static void
zmq_sd_socket_deinit(ZMQReaderContext* reader_context)
{
  log_pipe_unref((LogPipe *) reader_context->reader);
  zmq_ctx_term(reader_context->context);
  g_free(reader_context);
}

static gboolean
zmq_sd_deinit(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;
  GlobalConfig *cfg = log_pipe_get_config(&self->super.super.super);

  ZMQReaderContext* reader_context = g_new0(ZMQReaderContext, 1);
  reader_context->context = self->context;
  reader_context->reader = self->reader;
  if (self->reader)
  {
    log_pipe_deinit((LogPipe *) self->reader);
  }
  self->context = NULL;
  self->reader = NULL;

  gchar* persist_name = zmq_sd_get_persist_name(self);

  cfg_persist_config_add(cfg, persist_name, reader_context, (GDestroyNotify) zmq_sd_socket_deinit, FALSE);

  g_free(persist_name);

  return log_src_driver_deinit_method(s);
}

static void
zmq_sd_notify(LogPipe *s, gint notify_code, gpointer user_data)
{
  switch (notify_code)
    {
    case NC_CLOSE:
    case NC_READ_ERROR:
      {
        zmq_sd_deinit(s);
        break;
      }
    }
}

static void
zmq_sd_free(LogPipe *s)
{
  ZMQSourceDriver *self = (ZMQSourceDriver *) s;
  g_assert(!self->reader);
  log_reader_options_destroy(&self->reader_options);
  log_src_driver_free(s);
}

LogDriver *
zmq_sd_new(GlobalConfig *cfg)
{
  ZMQSourceDriver *self = g_new0(ZMQSourceDriver, 1);

  log_src_driver_init_instance(&self->super, cfg);

  self->super.super.super.init = zmq_sd_init;
  self->super.super.super.deinit = zmq_sd_deinit;
  self->super.super.super.notify = zmq_sd_notify;
  self->super.super.super.free_fn = zmq_sd_free;

  zmq_sd_set_address((LogDriver *) self, "localhost");
  zmq_sd_set_port((LogDriver *) self, 5558);

  log_reader_options_defaults(&self->reader_options);
  return &self->super.super;
}
