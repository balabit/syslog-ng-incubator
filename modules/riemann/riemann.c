/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Gergely Nagy <algernon@balabit.hu>
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

#include <riemann/riemann-client.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "logthrdestdrv.h"
#include "misc.h"
#include "stats.h"
#include "plugin.h"
#include "plugin-types.h"
#include "riemann.h"
#include "riemann-parser.h"

#ifndef SCS_RIEMANN
#define SCS_RIEMANN 0
#endif

typedef struct
{
  LogThrDestDriver super;

  gchar *server;
  gint port;
  gint on_error;

  struct
  {
    LogTemplate *host;
    LogTemplate *service;
    LogTemplate *state;
    LogTemplate *description;
    LogTemplate *metric;
    LogTemplate *ttl;
    GList *tags;
  } fields;

  GString *str;

  riemann_client_t *client;
} RiemannDestDriver;

/*
 * Configuration
 */
void
riemann_dd_set_server(LogDriver *d, const gchar *host)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  g_free(self->server);
  self->server = g_strdup(host);
}

void
riemann_dd_set_port(LogDriver *d, gint port)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  self->port = port;
}

void
riemann_dd_set_on_error(LogDriver *d, gint on_error)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  self->on_error = on_error;
}

void
riemann_dd_set_field_host(LogDriver *d, LogTemplate *value)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  self->fields.host = log_template_ref(value);
}

void
riemann_dd_set_field_service(LogDriver *d, LogTemplate *value)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  self->fields.service = log_template_ref(value);
}

void
riemann_dd_set_field_state(LogDriver *d, LogTemplate *value)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  self->fields.state = log_template_ref(value);
}

void
riemann_dd_set_field_description(LogDriver *d, LogTemplate *value)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  self->fields.description = log_template_ref(value);
}

void
riemann_dd_set_field_metric(LogDriver *d, LogTemplate *value)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  self->fields.metric = log_template_ref(value);
}

void
riemann_dd_set_field_ttl(LogDriver *d, LogTemplate *value)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  self->fields.ttl = log_template_ref(value);
}

void
riemann_dd_set_field_tags(LogDriver *d, GList *taglist)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  string_list_free(self->fields.tags);
  self->fields.tags = taglist;
}

/*
 * Utilities
 */

static gchar *
riemann_dd_format_stats_instance(RiemannDestDriver *self)
{
  static gchar persist_name[1024];

  g_snprintf(persist_name, sizeof(persist_name),
             "riemann,%s,%u", self->server, self->port);
  return persist_name;
}

static gchar *
riemann_dd_format_persist_name(RiemannDestDriver *self)
{
  static gchar persist_name[1024];

  g_snprintf(persist_name, sizeof(persist_name),
             "riemann(%s,%u)", self->server, self->port);
  return persist_name;
}

static void
riemann_dd_disconnect(LogThrDestDriver *s)
{
  RiemannDestDriver *self = (RiemannDestDriver *)s;

  riemann_client_disconnect(self->client);
  self->client = NULL;
}

static gboolean
riemann_dd_connect(RiemannDestDriver *self, gboolean reconnect)
{
  if (reconnect && self->client)
    return TRUE;

  self->client = riemann_client_create(RIEMANN_CLIENT_TCP,
                                       self->server, self->port);
  if (!self->client)
    {
      msg_error("Error connecting to Riemann",
                evt_tag_errno("errno", errno),
                NULL);
      return FALSE;
    }

  return TRUE;
}

/*
 * Main thread
 */

static gboolean
riemann_worker_init(LogPipe *s)
{
  RiemannDestDriver *self = (RiemannDestDriver *)s;
  GlobalConfig *cfg = log_pipe_get_config(s);

  if (!log_threaded_dest_driver_init_method(&self->super,
                                            riemann_dd_format_persist_name(self),
                                            SCS_RIEMANN, riemann_dd_format_stats_instance(self)))
    return FALSE;

  if (!self->server)
    self->server = g_strdup("127.0.0.1");
  if (self->port == -1)
    self->port = 5555;

  if (!self->fields.host)
    {
      self->fields.host = log_template_new(cfg, NULL);
      log_template_compile(self->fields.host, "${HOST}", NULL);
    }
  if (!self->fields.service)
    {
      self->fields.service = log_template_new (cfg, NULL);
      log_template_compile(self->fields.service, "${PROGRAM}", NULL);
    }

  if (self->on_error == 0)
    self->on_error = cfg->template_options.on_error;

  msg_verbose("Initializing Riemann destination",
              evt_tag_str("driver", self->super.super.super.id),
              evt_tag_str("server", self->server),
              evt_tag_int("port", self->port),
              NULL);

  log_threaded_dest_driver_start(&self->super);

  return TRUE;
}

static gboolean
riemann_worker_deinit(LogPipe *s)
{
  RiemannDestDriver *self = (RiemannDestDriver *)s;

  riemann_dd_disconnect(&self->super);

  return log_threaded_dest_driver_deinit_method(&self->super,
                                                SCS_RIEMANN,
                                                riemann_dd_format_stats_instance(self));
}

static void
riemann_dd_field_maybe_add(riemann_event_t *event, LogMessage *msg,
                           LogTemplate *template, riemann_event_field_t ftype,
                            GString *target)
{
  if (!template)
    return;

  log_template_format(template, msg, NULL, LTZ_SEND, 0,NULL, target);
  riemann_event_set(event, ftype, target->str, RIEMANN_EVENT_FIELD_NONE);
}

static void
riemann_dd_field_add_tag(gpointer data, gpointer user_data)
{
  gchar *tag = (gchar *)data;
  riemann_event_t *event = (riemann_event_t *)user_data;

  riemann_event_tag_add(event, tag);
}

static gboolean
riemann_worker_insert(LogThrDestDriver *s)
{
  RiemannDestDriver *self = (RiemannDestDriver *)s;
  riemann_event_t *event;
  gboolean success, need_drop = FALSE;
  LogMessage *msg;
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;

  riemann_dd_connect(self, TRUE);

  success = log_queue_pop_head(self->super.queue, &msg, &path_options, FALSE, FALSE);
  if (!success)
    return TRUE;

  msg_set_context(msg);

  event = riemann_event_new();

  if (self->fields.metric)
    {
      log_template_format(self->fields.metric, msg, NULL, LTZ_SEND,
                          0, NULL, self->str);

      switch (self->fields.metric->type_hint)
        {
        case TYPE_HINT_INT32:
        case TYPE_HINT_INT64:
          {
            glong i;

            if (type_cast_to_int64(self->str->str, &i, NULL))
              riemann_event_set(event, RIEMANN_EVENT_FIELD_METRIC_S64, i,
                                RIEMANN_EVENT_FIELD_NONE);
            else
              need_drop = type_cast_drop_helper(self->on_error,
                                                self->str->str, "int");
            break;
          }
        case TYPE_HINT_STRING:
          {
            gfloat f;
            gchar *endptr;
            gboolean r;

            errno = 0;
            f = strtof(self->str->str, &endptr);
            if ((errno == ERANGE && (f == HUGE_VAL || f == -HUGE_VAL))
                || (errno == 0 && f == 0))
              r = FALSE;
            if (endptr == self->str->str)
              r = FALSE;

            if (r)
              riemann_event_set(event, RIEMANN_EVENT_FIELD_METRIC_D, (double) f,
                                RIEMANN_EVENT_FIELD_NONE);
            else
              need_drop = type_cast_drop_helper(self->on_error,
                                                self->str->str, "float");
            break;
          }
        default:
          need_drop = type_cast_drop_helper(self->on_error,
                                            self->str->str, "<unknown>");
          break;
        }
    }

  if (!need_drop && self->fields.ttl)
    {
      float f;
      gchar *endptr;
      gboolean r = TRUE;

      log_template_format(self->fields.ttl, msg, NULL, LTZ_SEND, 0, NULL, self->str);

      errno = 0;
      f = strtof(self->str->str, &endptr);
      if ((errno == ERANGE && (f == HUGE_VAL || f == -HUGE_VAL))
          || (errno == 0 && f == 0))
        r = FALSE;
      if (endptr == self->str->str)
        r = FALSE;

      if (r)
        riemann_event_set(event, RIEMANN_EVENT_FIELD_TTL, f,
                          RIEMANN_EVENT_FIELD_NONE);
      else
        need_drop = type_cast_drop_helper(self->on_error,
                                          self->str->str, "float");
    }

  if (!need_drop)
    {
      riemann_dd_field_maybe_add(event, msg, self->fields.host,
                                 RIEMANN_EVENT_FIELD_HOST, self->str);
      riemann_dd_field_maybe_add(event, msg, self->fields.service,
                                 RIEMANN_EVENT_FIELD_SERVICE, self->str);
      riemann_dd_field_maybe_add(event, msg, self->fields.description,
                                 RIEMANN_EVENT_FIELD_DESCRIPTION, self->str);
      riemann_dd_field_maybe_add(event, msg, self->fields.state,
                                 RIEMANN_EVENT_FIELD_STATE, self->str);

      g_list_foreach(self->fields.tags, riemann_dd_field_add_tag,
                     (gpointer)event);

      riemann_client_send_message_oneshot
        (self->client,
         riemann_message_create_with_events(event, NULL));
    }

  msg_set_context(NULL);

  if (success)
    {
      stats_counter_inc(self->super.stored_messages);
      log_msg_ack(msg, &path_options);
      log_msg_unref(msg);
    }
  else
    {
      if (need_drop)
        {
          stats_counter_inc(self->super.dropped_messages);
          log_msg_ack(msg, &path_options);
          log_msg_unref(msg);
        }
      else
        log_queue_push_head(self->super.queue, msg, &path_options);
    }

  return success;
}

/*
 * Plugin glue.
 */

static void
riemann_worker_thread_init(LogThrDestDriver *d)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  riemann_dd_connect(self, FALSE);
}

static void
riemann_worker_thread_deinit (LogThrDestDriver *d)
{
  riemann_dd_disconnect(d);
}

static void
riemann_dd_free(LogPipe *d)
{
  RiemannDestDriver *self = (RiemannDestDriver *)d;

  g_free(self->server);

  riemann_client_free(self->client);
  g_string_free(self->str, TRUE);

  log_template_unref(self->fields.host);
  log_template_unref(self->fields.service);
  log_template_unref(self->fields.state);
  log_template_unref(self->fields.description);
  log_template_unref(self->fields.metric);
  log_template_unref(self->fields.ttl);
  string_list_free(self->fields.tags);

  log_threaded_dest_driver_free(d);
}

LogDriver *
riemann_dd_new(void)
{
  RiemannDestDriver *self = g_new0(RiemannDestDriver, 1);

  log_threaded_dest_driver_init_instance(&self->super);

  self->super.super.super.super.init = riemann_worker_init;
  self->super.super.super.super.deinit = riemann_worker_deinit;
  self->super.super.super.super.free_fn = riemann_dd_free;

  self->super.worker.init = riemann_worker_thread_init;
  self->super.worker.deinit = riemann_worker_thread_deinit;
  self->super.worker.disconnect = riemann_dd_disconnect;
  self->super.worker.insert = riemann_worker_insert;

  self->port = -1;

  self->str = g_string_sized_new (1024);

  return (LogDriver *)self;
}

extern CfgParser riemann_dd_parser;

static Plugin riemann_plugin =
{
  .type = LL_CONTEXT_DESTINATION,
  .name = "riemann",
  .parser = &riemann_parser,
};

gboolean
riemann_module_init(GlobalConfig *cfg, CfgArgs *args G_GNUC_UNUSED)
{
  plugin_register(cfg, &riemann_plugin, 1);
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "riemann",
  .version = VERSION,
  .description = "The riemann module provides Riemann destination support for syslog-ng.",
  .core_revision = VERSION_CURRENT_VER_ONLY,
  .plugins = &riemann_plugin,
  .plugins_len = 1,
};
