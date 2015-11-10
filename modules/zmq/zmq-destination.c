/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Laszlo Meszaros <lmesz@balabit.hu>
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

#ifndef SCS_ZMQ
#define SCS_ZMQ 0
#endif

void zmq_dd_set_address(LogDriver *d, const gchar *address);
void zmq_dd_set_port(LogDriver *d, gint port);
void zmq_dd_set_template(LogDriver *d, gchar *template);

void
zmq_dd_set_address(LogDriver *d, const gchar *address){
    ZMQDestDriver *self = (ZMQDestDriver *)d;
    g_free(self->address);
    self->address = g_strdup(address);
}

void
zmq_dd_set_port(LogDriver *d, gint port)
{
  ZMQDestDriver *self = (ZMQDestDriver *)d;
  self->port = port;
}

void
zmq_dd_set_template(LogDriver *d, gchar *template)
{
  ZMQDestDriver *self = (ZMQDestDriver *)d;
  GlobalConfig* cfg = log_pipe_get_config(&d->super);
  self->template = log_template_new(cfg, NULL);
  log_template_compile(self->template, template, NULL);
}

LogTemplateOptions *
zmq_dd_get_template_options(LogDriver *d)
{
  ZMQDestDriver *self = (ZMQDestDriver *)d;
  return &self->template_options;
}

static gchar *
zmq_dd_format_stats_instance(LogThrDestDriver *d)
{
  static gchar persist_name[1024];

  g_snprintf(persist_name, sizeof(persist_name), "zmq()");
  return persist_name;
}

static gchar *
zmq_dd_format_persist_name(LogThrDestDriver *d)
{
  static gchar persist_name[1024];

  g_snprintf(persist_name, sizeof(persist_name), "zmq()");
  return persist_name;
}

gchar *
zmq_dd_get_address(ZMQDestDriver* self)
{
  return g_strdup_printf("tcp://%s:%d", self->address, self->port);
}

gboolean
zmq_dd_connect(ZMQDestDriver *self)
{
  gboolean result = TRUE;
  self->context = zmq_ctx_new();
  self->socket = zmq_socket(self->context, ZMQ_PUSH);

  gchar *connection_string = zmq_dd_get_address(self);

  if (zmq_bind(self->socket, connection_string) == 0)
  {
    msg_verbose("Succesfully bind!", evt_tag_int("Port", self->port), NULL);
    result = TRUE;
  }
  else
  {
    msg_verbose("Failed to bind!", evt_tag_int("Port", self->port), NULL);
    result = FALSE;
  }

  g_free(connection_string);
  return result;
}

static void
zmq_dd_disconnect(LogThrDestDriver *d)
{
  ZMQDestDriver *self = (ZMQDestDriver *)d;
  zmq_close(self->socket);
  zmq_ctx_destroy(self->context);
}

static worker_insert_result_t
zmq_worker_insert(LogThrDestDriver *d, LogMessage *msg)
{
  ZMQDestDriver *self = (ZMQDestDriver *)d;
  GString *result = g_string_new("");

  if (self->socket == NULL)
  {
    if (!zmq_dd_connect(self))
    {
      return WORKER_INSERT_RESULT_ERROR;
    }
  }

  log_template_format(self->template, msg, NULL, LTZ_LOCAL, self->seq_num, NULL, result);

  if (zmq_send (self->socket, result->str, result->len, 0) == -1)
  {
    msg_error("Failed to add message to zmq queue!", evt_tag_errno("errno", errno), NULL);
    g_string_free(result, TRUE);
    return WORKER_INSERT_RESULT_ERROR;
  }
  else
  {
    g_string_free(result, TRUE);
    return WORKER_INSERT_RESULT_SUCCESS;
  }
}

static void
zmq_worker_thread_init(LogThrDestDriver *d)
{
  ZMQDestDriver *self = (ZMQDestDriver *)d;

  msg_debug("Worker thread started",
            evt_tag_str("driver", self->super.super.super.id),
            NULL);

  zmq_dd_connect(self);
}

gboolean
zmq_dd_init(LogPipe *d)
{
  ZMQDestDriver *self = (ZMQDestDriver *)d;
  GlobalConfig *cfg = log_pipe_get_config(d);

  log_template_options_init(&self->template_options, cfg);

  log_dest_driver_init_method(d);

  msg_verbose("Initializing ZeroMQ destination",
              evt_tag_str("driver", self->super.super.super.id),
              NULL);

  log_threaded_dest_driver_start(d);
  return TRUE;
}

static void
zmq_dd_free(LogPipe *d)
{
  log_threaded_dest_driver_free(d);
}

LogDriver *
zmq_dd_new(GlobalConfig *cfg)
{
  ZMQDestDriver *self = g_new0(ZMQDestDriver, 1);

  log_threaded_dest_driver_init_instance(&self->super, cfg);
  self->super.super.super.super.init = zmq_dd_init;
  self->super.super.super.super.free_fn = zmq_dd_free;

  self->super.worker.thread_init = zmq_worker_thread_init;
  self->super.worker.disconnect = zmq_dd_disconnect;
  self->super.worker.insert = zmq_worker_insert;

  self->super.format.stats_instance = zmq_dd_format_stats_instance;
  self->super.format.persist_name = zmq_dd_format_persist_name;
  self->super.stats_source = SCS_ZMQ;

  zmq_dd_set_port((LogDriver *) self, 5556);
  zmq_dd_set_template((LogDriver *) self, "${MESSAGE}");

  init_sequence_number(&self->seq_num);

  return (LogDriver *)self;
}
