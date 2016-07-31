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

#ifndef WEBSOCKET_H_INCLUDED
#define WEBSOCKET_H_INCLUDED
#define SCS_WEBSOCKET 0

#include "driver.h"
#include "logreader.h"

typedef struct _WebsocketSrcDriver
{
  LogSrcDriver super;
  LogReader* reader;
  LogReaderOptions reader_options;

  int port;
  gchar *address;
  gchar *protocol;
  gchar *path;
  gchar *cert;
  gchar *key;
  gchar *cacert;
  int enable_ssl;
  int client_use_ssl_flag;
} WebsocketSrcDriver;

LogDriver *websocket_sd_new(GlobalConfig *cfg);
void websocket_sd_set_port(LogDriver *driver, gint port);
void websocket_sd_set_address(LogDriver *dirver, gchar *address);
void websocket_sd_set_protocol(LogDriver *dirver, gchar *protocol);
void websocket_sd_set_path(LogDriver *dirver, gchar *path);
void websocket_sd_set_cert(LogDriver *dirver, gchar *path);
void websocket_sd_set_key(LogDriver *dirver, gchar *path);
void websocket_sd_set_cacert(LogDriver *dirver, gchar *path);
void websocket_sd_set_enable_ssl(LogDriver *dirver, int flag);
void websocket_sd_set_allow_self_signed(LogDriver *dirver, int flag);

#endif
