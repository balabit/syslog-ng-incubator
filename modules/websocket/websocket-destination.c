/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2016 Yilin Li <liyilin1214@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "websocket-destination.h"
#include "client.h"
#include <stdio.h>


gboolean
websocket_dd_init(LogPipe *destination)
{
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  GlobalConfig *cfg = log_pipe_get_config(destination);

  if (!log_dest_driver_init_method(destination))
    return FALSE;

  log_template_options_init(&self->template_options, cfg);

  if (self->enable_ssl)
    self->client_use_ssl_flag = self->allow_self_signed ? 2 : 1;
  else
    self->client_use_ssl_flag = 0;
  return log_threaded_dest_driver_start(destination);
}

LogDriver *websocket_dd_new(GlobalConfig *cfg);

void websocket_dd_set_port(LogDriver *destination, gint port) {
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  self->port = port;
}

void
websocket_dd_set_address(LogDriver *destination, gchar *address) {
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  g_free(self->address);
  self->address = g_strdup(address);
}

void
websocket_dd_set_protocol(LogDriver *destination, gchar *protocol) {
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  g_free(self->protocol);
  self->protocol = g_strdup(protocol);
}

void
websocket_dd_set_path(LogDriver *destination, gchar *path) {
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  g_free(self->path);
  self->path = g_strdup(path);
}

void websocket_dd_set_enable_ssl(LogDriver *destination, int flag) {
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  self->enable_ssl = flag;
}

void websocket_dd_set_cert(LogDriver *destination, gchar *path) {
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  g_free(self->cert);
  self->cert = path == NULL ? NULL : g_strdup(path);
}

void websocket_dd_set_key(LogDriver *destination, gchar *path) {
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  g_free(self->key);
  self->key = path == NULL ? NULL : g_strdup(path);
}

void websocket_dd_set_cacert(LogDriver *destination, gchar *path) {
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  g_free(self->cacert);
  self->cacert = path == NULL ? NULL : g_strdup(path);
}

void websocket_dd_set_allow_self_signed(LogDriver *destination, int flag) {
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  self->allow_self_signed = flag;
}

void
websocket_dd_set_template(LogDriver *destination, gchar *template)
{
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  GlobalConfig* cfg = log_pipe_get_config(&destination->super);
  g_free(self->template);
  self->template = log_template_new(cfg, NULL);
  log_template_compile(self->template, template, NULL);
}

LogTemplateOptions *
destination_dd_get_template_options(LogDriver *d)
{
  WebsocketDestDriver *self = (WebsocketDestDriver *)d;
  return &self->template_options;
}

static void
websocket_dd_free(LogPipe *destination)
{
  WebsocketDestDriver *self = (WebsocketDestDriver *) destination;
  g_free(self->template);
  g_free(self->address);
  g_free(self->path);
  g_free(self->protocol);
  g_free(self->cert);
  g_free(self->key);
  g_free(self->cacert);
  log_threaded_dest_driver_free(destination);
}


static void
websocket_worker_thread_init(LogThrDestDriver *destination)
{
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;

  msg_debug("Worker thread started",
            evt_tag_str("driver", self->super.super.super.id),
            NULL);

  websocket_client_create(self->protocol, self->address, self->port,
    self->path, self->client_use_ssl_flag, self->cert, self->key, self->cacert);
}


static void
websocket_dd_disconnect(LogThrDestDriver *destination)
{
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  websocket_client_disconnect();
}

static worker_insert_result_t
websocket_worker_insert(LogThrDestDriver *destination, LogMessage *msg)
{
  WebsocketDestDriver *self = (WebsocketDestDriver *)destination;
  GString *result = g_string_new("");
  log_template_format(self->template, msg, &self->template_options, LTZ_LOCAL, self->super.seq_num, NULL, result);
  websocket_client_send_msg(result->str);
  g_string_free(result, TRUE);
  return WORKER_INSERT_RESULT_SUCCESS;;
}

static gchar *
websocket_dd_format_stats_instance(LogThrDestDriver *d)
{
  static gchar persist_name[1024];
  WebsocketDestDriver *self = (WebsocketDestDriver *)d;

  g_snprintf(persist_name, sizeof(persist_name), "websocket:%s:%d:%s:%s",
    self->address, self->port, self->protocol, self->path);
  return persist_name;
}

static gchar *
websocket_dd_format_persist_name(LogThrDestDriver *d)
{
  static gchar persist_name[1024];
  WebsocketDestDriver *self = (WebsocketDestDriver *)d;

  g_snprintf(persist_name, sizeof(persist_name), "websocket:%s:%d:%s:%s",
    self->address, self->port, self->protocol, self->path);
  return persist_name;
}

LogTemplateOptions *
websocket_dd_get_template_options(LogDriver *d)
{
  WebsocketDestDriver *self = (WebsocketDestDriver *)d;
  return &self->template_options;
}

LogDriver *
websocket_dd_new(GlobalConfig *cfg)
{
  WebsocketDestDriver *self = g_new0(WebsocketDestDriver, 1);

  log_threaded_dest_driver_init_instance(&self->super, cfg);
  self->super.super.super.super.init = websocket_dd_init;
  self->super.super.super.super.free_fn = websocket_dd_free;

  self->super.worker.thread_init = websocket_worker_thread_init;
  self->super.worker.disconnect = websocket_dd_disconnect;
  self->super.worker.insert = websocket_worker_insert;

  self->super.format.stats_instance = websocket_dd_format_stats_instance;
  self->super.format.persist_name = websocket_dd_format_persist_name;
  self->super.stats_source = SCS_WEBSOCKET;

  websocket_dd_set_protocol((LogDriver *) self, "");
  websocket_dd_set_port((LogDriver *) self, 7681);
  websocket_dd_set_address((LogDriver *) self, "127.0.0.1");
  websocket_dd_set_path((LogDriver *) self, "/");
  websocket_dd_set_enable_ssl((LogDriver *) self, FALSE);
  websocket_dd_set_cert((LogDriver *) self, NULL);
  websocket_dd_set_key((LogDriver *) self, NULL);
  websocket_dd_set_cacert((LogDriver *) self, NULL);
  websocket_dd_set_template((LogDriver *) self, "${MESSAGE}");
  websocket_dd_set_allow_self_signed((LogDriver *) self, FALSE);

  return (LogDriver *)self;
}
