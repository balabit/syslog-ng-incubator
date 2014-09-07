/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Tihamer Petrovics <tihameri@gmail.com>
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
#define SCS_ZMQ 0

#include "driver.h"
#include "logreader.h"

typedef struct _ZMQSourceDriver
{
  LogSrcDriver super;
  LogReader* reader;
  LogReaderOptions reader_options;

  void* context;
  void* socket;

  gchar *address;
  gint port;

} ZMQSourceDriver;

void zmq_sd_set_address(LogDriver *source, const gchar *address);
void zmq_sd_set_port(LogDriver *source, gint port);
LogDriver *zmq_sd_new();

#endif
