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

#include "websocket-transport.h"
#include <stdio.h>
#include <unistd.h>

static gssize
log_transport_websocket_read_method(LogTransport *s, gpointer buf, gsize buflen, LogTransportAuxData *aux)
{
  printf("calling: log_transport_websocket_read_method   buflen=%d\n", buflen);
  return read(STDIN_FILENO, buf, buflen);
}

static gssize
log_transport_websocket_write_method(LogTransport *s, const gpointer buf, gsize buflen)
{
  printf("calling: log_transport_websocket_write_method buflen=%d\n", buflen);
  return 0;
}

static void
log_transport_websocket_free_method(LogTransport *s)
{
  // TODO: Maybe we should free websocket here
  return;
}

LogTransport *
log_transport_websocket_new()
{
  LogTransport *self = g_new0(LogTransport, 1);
  int fd = STDIN_FILENO;  //TODO: we just use stdout first to test
  size_t fd_size = sizeof(fd);

  log_transport_init_instance(self, fd);

  self->read = log_transport_websocket_read_method;
  self->write = log_transport_websocket_write_method;
  self->free_fn = log_transport_websocket_free_method;

  return self;
}
