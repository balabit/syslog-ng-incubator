/*
 * Copyright (c) 2002-2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 1998-2013 Balázs Scheidler
 * Copyright (c) 2013 Tusa Viktor
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


#include "rss.h"
#include "rss-parser.h"
#include "plugin.h"
#include "plugin-types.h"
#include "messages.h"

static GString *
_format_feed_header(RssDestDriver * self, GString * result)
{
  result =
    g_string_append (result,
		     "<?xml version=\"1.0\"?>\n<feed xmlns=\"http://www.w3.org/2005/Atom\">\n<title>");
  result = g_string_append (result, self->feed_title->str);
  result = g_string_append (result, "</title><link>");
  result = g_string_append (result, self->address->str);
  result = g_string_append (result, "</link>");
  return result;
}

static GString* 
_format_feed_footer(RssDestDriver * self, GString * result)
{
  return g_string_append (result, "</feed>\n");
};

GString *
_format_backlog (RssDestDriver * self, GString * result)
{
  GList *feed_item = self->backlog;
  GString *message;
  char id_str[10];
  int offset = self->id;
  result = _format_feed_header(self, result);
  while (feed_item)
    {
      message = g_string_new ("");
      log_template_format (self->entry_title, feed_item->data, NULL, LTZ_LOCAL, 0,
			   NULL, message);
      result = g_string_append (result, "<entry>\n <title>");
      result = g_string_append (result, message->str);
      result = g_string_append (result, "</title>\n <description>");
      log_template_format (self->entry_description, feed_item->data, NULL, LTZ_LOCAL, 0,
			   NULL, message);
      result = g_string_append (result, message->str);
      result = g_string_append (result, "</description>\n <id>");
      snprintf (id_str, 10, "%d", offset);
      result = g_string_append (result, id_str);
      result = g_string_append (result, "</id>\n</entry>\n");
      g_string_free (message, TRUE);
      feed_item = g_list_next (feed_item);
      offset++;
    }
  result = _format_feed_footer(self, result); 
  return result;
}

static void _generate_address_string(RssDestDriver* self)
{
  self->address = g_string_new("");
  g_string_printf(self->address, "localhost:%d", self->port);
}

static int
_init_rss_listen_socket (RssDestDriver * self)
{
  int sock, optval;
  struct sockaddr_in lsockaddr;

  sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  optval = 1;
  if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) ==
      -1)
    msg_error ("RSS setsockopt failed!", NULL);

  lsockaddr.sin_family = AF_INET;
  lsockaddr.sin_port = htons (self->port);
  lsockaddr.sin_addr.s_addr = INADDR_ANY;
  memset (lsockaddr.sin_zero, '\0', sizeof (lsockaddr.sin_zero));

  if (bind (sock, (struct sockaddr *) &lsockaddr, sizeof (lsockaddr)) < 0)
    {
      msg_error ("RSS Bind failed!", NULL);
      return -1;
    }
  if (listen (sock, 10) < 0)
    {
      msg_error ("RSS Listen failed!", NULL);
      return -1;
    }
  return sock;

}

static int
_accept_rss_connection (int sock)
{
  struct sockaddr_in rsockaddr;
  int addrlen;
  return accept (sock, (struct sockaddr*)&rsockaddr, &addrlen);
}

static GString *
_create_http_header ()
{
  return
    g_string_new ("HTTP/1.1 200 OK\nContent-Type:application/atom+xml\n\n");
}

static void
_respond_with_feed (int sock, GString * feed)
{
  gchar buffer[4096];

  read (sock, buffer, sizeof (buffer));
  write (sock, feed->str, feed->len);
}

static void
_accept_and_serve_rss_connection (void* userdata)
{
  GString *feed;
  int newsock, sock;
  RssDestDriver * self = (RssDestDriver*) userdata;
  sock = self->listen_fd.fd;

  newsock = _accept_rss_connection (sock);
  feed = _create_http_header ();
  feed = _format_backlog (self, feed);
  _respond_with_feed (newsock, feed);

  shutdown (newsock, 2);
  close (newsock);
  g_string_free (feed, TRUE);
}

static void
_remove_msg_from_backlog_unlocked (RssDestDriver * self)
{
  LogMessage *msg;

  msg = g_list_nth_data (self->backlog, 0);
  self->backlog = g_list_remove (self->backlog, msg);

  log_msg_unref (msg);
  self->id++;
}

static void
_append_msg_to_backlog (RssDestDriver * self, LogMessage * msg)
{
  g_mutex_lock (self->lock);

  log_msg_ref (msg);
  self->backlog = g_list_append (self->backlog, msg);

  if (g_list_length (self->backlog) > 100)
    {
      _remove_msg_from_backlog_unlocked (self);
    }

  g_mutex_unlock (self->lock);
}

static void
rss_dd_queue (LogPipe * s, LogMessage * msg,
	      const LogPathOptions * path_options, gpointer user_data)
{
  RssDestDriver *self = (RssDestDriver *) s;
  _append_msg_to_backlog (self, msg);
  log_dest_driver_queue_method (s, msg, path_options, user_data);
}

void
rss_dd_set_port (LogDriver * s, int port)
{
  RssDestDriver *self = (RssDestDriver *) s;
  self->port = port;
}

void 
rss_dd_set_title(LogDriver* s, const char* title)
{
  RssDestDriver *self = (RssDestDriver *) s;
  self->feed_title = g_string_new (g_strdup (title) );
}

void 
rss_dd_set_entry_title(LogDriver* s, const char* title)
{
  GError* error = NULL;
  RssDestDriver *self = (RssDestDriver *) s;

  self->entry_title = log_template_new (configuration, NULL);
  log_template_compile (self->entry_title, title, &error);
}

void 
rss_dd_set_entry_description(LogDriver* s, const char* description)
{
  GError* error = NULL;
  RssDestDriver *self = (RssDestDriver *) s;

  self->entry_description = log_template_new (configuration, NULL);
  log_template_compile (self->entry_description, description, &error);
}

static gboolean
_register_listen_fd (RssDestDriver * self)
{
  int listen_socket;
  IV_FD_INIT (&self->listen_fd);
  listen_socket = _init_rss_listen_socket (self);

  if (listen_socket == -1)
    return FALSE;

  self->listen_fd.fd = listen_socket;
  self->listen_fd.cookie = self;
  self->listen_fd.handler_in = _accept_and_serve_rss_connection;
  iv_fd_register (&self->listen_fd);
   
  _generate_address_string(self); 

  return TRUE;
}

gboolean
rss_dd_init (LogPipe * s)
{
  GError *error = NULL;
  RssDestDriver *self = (RssDestDriver *) s;
  if (!self->feed_title || !self->entry_title || !self->entry_description)
  {
     msg_error("title, entry_title, entry_description options are mandatory for RSS destination", NULL);
     return FALSE;
  }

  self->lock = g_mutex_new ();
  return _register_listen_fd (self);
}

gboolean
rss_dd_deinit (LogPipe * s)
{
  return TRUE;
}

void
rss_dd_free (LogPipe * s)
{
  RssDestDriver *self = (RssDestDriver *) s;
  g_list_free_full (self->backlog, (GDestroyNotify)log_msg_unref);
  g_string_free(self->address, TRUE);
  g_string_free(self->feed_title, TRUE);
  log_template_unref(self->entry_title);
  log_template_unref(self->entry_description);
}

LogDriver *
rss_dd_new (void)
{
  RssDestDriver *self = g_new0 (RssDestDriver, 1);
  log_dest_driver_init_instance (&self->super);
  self->super.super.super.queue = rss_dd_queue;
  self->super.super.super.init = rss_dd_init;
  self->super.super.super.deinit = rss_dd_deinit;
  self->super.super.super.free_fn = rss_dd_free;
  self->backlog = NULL;
  return &self->super.super;
}

extern CfgParser rss_dd_parser;

static Plugin rss_plugin = {
  .type = LL_CONTEXT_DESTINATION,
  .name = "rss",
  .parser = &rss_parser,
};

gboolean
rss_module_init (GlobalConfig * cfg, CfgArgs * args)
{
  plugin_register (cfg, &rss_plugin, 1);
  return TRUE;
}

const ModuleInfo module_info = {
  .canonical_name = "rss",
  .version = VERSION,
  .description =
    "The rss module is a destination driver to offer logs in RSS feed.",
  .core_revision = VERSION_CURRENT_VER_ONLY,
  .plugins = &rss_plugin,
  .plugins_len = 1,
};
