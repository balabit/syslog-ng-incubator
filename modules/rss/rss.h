/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
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

#ifndef RSS_H_INCLUDED
#define RSS_H_INCLUDED

#include "driver.h"
#include <iv.h>

typedef struct
{
  LogDestDriver super;
  GList* backlog;
  int port;
  int id;
  GMutex* lock;
  LogTemplate* entry_title;
  LogTemplate* entry_description;
  struct iv_fd listen_fd;
  GString *address;
  GString* feed_title;
} RssDestDriver;

LogDriver *rss_dd_new(void);
void rss_dd_set_port(LogDriver *self, int port);
void rss_dd_set_localip(LogDriver *s, gchar *localip);
void rss_dd_set_title(LogDriver *s, const char *title);
void rss_dd_set_entry_title(LogDriver *s, const char *title);
void rss_dd_set_entry_description(LogDriver *s, const char *title);

#endif
