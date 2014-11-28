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

#ifndef ZMQ_H_INCLUDED
#define ZMQ_H_INCLUDED

#include "driver.h"
#include "logthrdestdrv.h"

typedef struct
{
  LogThrDestDriver super;
  gchar *port;
  int socket_type;

  GlobalConfig *cfg;

  LogTemplateOptions template_options;
  LogTemplate *template;

  void *context;
  void *socket;

  gint32 seq_num;
} ZMQDestDriver;

LogDriver *zmq_dd_new(GlobalConfig *cfg);

void zmq_dd_set_port(LogDriver *destination, gint port);
gboolean zmq_dd_set_socket_type(LogDriver *destination, gchar *socket_type);
void zmq_dd_set_template(LogDriver *destination, gchar *template);
LogTemplateOptions *zmq_dd_get_template_options(LogDriver *destination);

#endif
