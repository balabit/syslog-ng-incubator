/*
 * Copyright (c) 2015 BalaBit
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

#include "parser.h"

struct NativeParserProxy;

__attribute__((visibility("hidden"))) void
native_parser_proxy_free(struct NativeParserProxy* this);

__attribute__((visibility("hidden"))) void
native_parser_proxy_set_option(struct NativeParserProxy* self, const gchar* key, const gchar* value);

__attribute__((visibility("hidden"))) gboolean
native_parser_proxy_process(struct NativeParserProxy* this, LogMessage *pmsg, const gchar *input, gsize input_len);

__attribute__((visibility("hidden"))) int
native_parser_proxy_init(struct NativeParserProxy* s);

__attribute__((visibility("hidden"))) struct NativeParserProxy*
native_parser_proxy_new(LogParser *super);

__attribute__((visibility("hidden"))) struct NativeParserProxy*
native_parser_proxy_clone(struct NativeParserProxy *self);

typedef struct _ParserNative {
  LogParser super;
  struct NativeParserProxy* native_object;
} ParserNative;

static LogPipe*
native_parser_clone(LogPipe *s);

static gboolean
native_parser_process(LogParser *s, LogMessage **pmsg, const LogPathOptions *path_options, const gchar *input, gsize input_len)
{
  ParserNative *self = (ParserNative *) s;

  LogMessage *writable_msg = log_msg_make_writable(pmsg, path_options);
  return native_parser_proxy_process(self->native_object, writable_msg, input, input_len);
}

void
native_parser_set_option(LogParser *s, gchar* key, gchar* value)
{    
  ParserNative *self = (ParserNative *)s;

  native_parser_proxy_set_option(self->native_object, key, value);
}

static gboolean
native_parser_init(LogPipe *s)
{
  ParserNative *self = (ParserNative *) s;

  return !!native_parser_proxy_init(self->native_object);
}

static void
native_parser_free(LogPipe *s)
{
  ParserNative *self = (ParserNative *)s;

  native_parser_proxy_free(self->native_object);
  log_parser_free_method(s);
}

static void
__setup_callback_methods(ParserNative *self)
{
  self->super.process = native_parser_process;
  self->super.super.free_fn = native_parser_free;
  self->super.super.clone = native_parser_clone;
  self->super.super.init = native_parser_init;
}

static LogPipe*
native_parser_clone(LogPipe *s)
{
  ParserNative *self = (ParserNative*) s;
  ParserNative *cloned;
  
  cloned = (ParserNative*) g_new0(ParserNative, 1);
  log_parser_init_instance(&cloned->super, s->cfg);
  cloned->native_object = native_parser_proxy_clone(self->native_object);

  if (!cloned->native_object)
    return NULL;

  __setup_callback_methods(cloned);
  
  return &cloned->super.super; 
}

__attribute__((visibility("hidden"))) LogParser*
native_parser_new(GlobalConfig *cfg)
{
  ParserNative *self = (ParserNative*) g_new0(ParserNative, 1);

  log_parser_init_instance(&self->super, cfg);
  self->native_object = native_parser_proxy_new(&self->super);

  if (!self->native_object)
    return NULL;

  __setup_callback_methods(self);

  return &self->super;
}
