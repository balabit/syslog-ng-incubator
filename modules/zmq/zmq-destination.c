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

#include "zmq-destination.h"
#include "zmq-parser.h"
#include "plugin.h"
#include "messages.h"
#include "misc.h"
#include "stats/stats.h"
#include "logqueue.h"
#include "driver.h"
#include "plugin-types.h"
#include "logthrdestdrv.h"

#ifndef SCS_ZMQ
#define SCS_ZMQ 0
#endif

void zmq_dd_set_port(LogDriver *destination, gint port);
gboolean zmq_dd_set_socket_type(LogDriver *destination, gchar *socket_type);
void zmq_dd_set_template(LogDriver *destination, gchar *template);

/*
 * Configuration
 */
void
zmq_dd_set_port(LogDriver *destination, gint port)
{
  ZMQDestDriver *self = (ZMQDestDriver *)destination;
  self->port = g_strdup_printf("%d", port);
}

gboolean
zmq_dd_set_socket_type(LogDriver *destination, gchar *socket_type)
{
  ZMQDestDriver *self = (ZMQDestDriver *)destination;

  // ZMQ_PUB, ZMQ_REQ, ZMQ_PUSH
  if (strcmp(socket_type, "publish") == 0)
    self->socket_type = ZMQ_PUB;
  else if (strcmp(socket_type, "request") == 0)
    self->socket_type = ZMQ_REQ;
  else if (strcmp(socket_type, "push") == 0)
    self->socket_type = ZMQ_PUSH;
  else
    return FALSE;

  return TRUE;
}

void
zmq_dd_set_template(LogDriver *destination, gchar *template)
{
  ZMQDestDriver *self = (ZMQDestDriver *)destination;
  GlobalConfig* cfg = log_pipe_get_config(&destination->super);
  self->template = log_template_new(cfg, NULL);
  log_template_compile(self->template, template, NULL);
}

LogTemplateOptions *
zmq_dd_get_template_options(LogDriver *d)
{
  ZMQDestDriver *self = (ZMQDestDriver *)d;
  return &self->template_options;
}

/*
 * Utilities
 */

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

static gboolean
zmq_dd_connect(ZMQDestDriver *self, gboolean reconnect)
{
  gboolean bind_result = TRUE;
  self->context = zmq_ctx_new();
  self->socket = zmq_socket(self->context, self->socket_type);

  gchar *connection_string = g_strconcat("tcp://*:", self->port, NULL);

  if (zmq_bind(self->socket, connection_string) == 0)
  {
    msg_verbose("Succesfully bind!", evt_tag_str("Port", self->port), NULL);
    bind_result = TRUE;
  }
  else
  {
    msg_verbose("Failed to bind!", evt_tag_str("Port", self->port), NULL);
    bind_result = FALSE;
  }

  g_free(connection_string);
  return bind_result;
}

static void
zmq_dd_disconnect(LogThrDestDriver *destination)
{
  ZMQDestDriver *self = (ZMQDestDriver *)destination;
  zmq_close(self->socket);
  zmq_ctx_destroy(self->context);
}

/*
 * Worker thread
 */

static worker_insert_result_t
zmq_worker_insert(LogThrDestDriver *destination, LogMessage *msg)
{
  ZMQDestDriver *self = (ZMQDestDriver *)destination;
  gboolean success = TRUE;
  GString *result = g_string_new("");

  if (self->socket == NULL)
  {
    if (!zmq_dd_connect(self, FALSE))
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
    return WORKER_INSERT_RESULT_SUCCESS;;
  }
}

static void
zmq_worker_thread_init(LogThrDestDriver *destination)
{
  ZMQDestDriver *self = (ZMQDestDriver *)destination;

  msg_debug("Worker thread started",
            evt_tag_str("driver", self->super.super.super.id),
            NULL);

  zmq_dd_connect(self, FALSE);
}

/*
 * Main thread
 */

gboolean
zmq_dd_init(LogPipe *destination)
{
  ZMQDestDriver *self = (ZMQDestDriver *)destination;
  GlobalConfig *cfg = log_pipe_get_config(destination);

  log_template_options_init(&self->template_options, cfg);

  log_dest_driver_init_method(destination);

  msg_verbose("Initializing ZeroMQ destination",
              evt_tag_str("driver", self->super.super.super.id),
              NULL);

  log_threaded_dest_driver_start(destination);
  return TRUE;
}

static void
zmq_dd_free(LogPipe *destination)
{
  log_threaded_dest_driver_free(destination);
}

/*
 * Plugin glue.
 */

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
  zmq_dd_set_socket_type((LogDriver *) self, "push");
  zmq_dd_set_template((LogDriver *) self, "${MESSAGE}");

  init_sequence_number(&self->seq_num);

  return (LogDriver *)self;
}
