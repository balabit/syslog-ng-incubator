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

#ifndef WEBSOCKET_H_INCLUDED
#define WEBSOCKET_H_INCLUDED

#include "driver.h"
#include "logthrdestdrv.h"
#define SCS_WEBSOCKET 0

typedef struct
{
  LogThrDestDriver super;
  int port;
  gchar *address;
  gchar *protocol;
  gchar *path;
  LogTemplateOptions template_options;
  LogTemplate *template;
} WebsocketDestDriver;

LogDriver *websocket_dd_new(GlobalConfig *cfg);
void websocket_dd_set_port(LogDriver *driver, gint port);
void websocket_dd_set_address(LogDriver *dirver, gchar *address);
void websocket_dd_set_protocol(LogDriver *dirver, gchar *protocol);
void websocket_dd_set_path(LogDriver *dirver, gchar *path);
LogTemplateOptions * websocket_dd_get_template_options(LogDriver *d);
#endif
