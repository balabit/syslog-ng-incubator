/*
 * Copyright (c) 2013, 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013, 2014 Viktor Tusa <tusa@balabit.hu>
 * Copyright (c) 2014 Gergely Nagy <algernon@balabit.hu>
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

#ifndef _AFLUA_DEST_H
#define _AFLUA_DEST_H

#include "driver.h"
#include "logwriter.h"
#include "value-pairs.h"
#include "logthrdestdrv.h"
#include <lua.h>

typedef struct _LuaDestDriver
{
  LogThrDestDriver super;
  lua_State *state;
  gchar *template_string;
  gchar *filename;
  gchar *init_func_name;
  gchar *queue_func_name;
  gchar *deinit_func_name;
  LogTemplate *template;
  LogTemplateOptions template_options;
  gint mode;
  ValuePairs *params;
  GList *globals;
} LuaDestDriver;

LogDriver *lua_dd_new();
void lua_dd_set_init_func(LogDriver *d, gchar *init_func_name);
void lua_dd_set_queue_func(LogDriver *d, gchar *queue_func_name);
void lua_dd_set_deinit_func(LogDriver *d, gchar *deinit_func_name);
void lua_dd_set_filename(LogDriver *d, gchar *filename);
void lua_dd_set_template(LogDriver *d, LogTemplate *template);
void lua_dd_set_mode(LogDriver *d, gchar *mode);
void lua_dd_set_params(LogDriver *d, ValuePairs *vp);
void lua_dd_add_global_constant(LogDriver *d, const char *name, const char *value);
void lua_dd_add_global_constant_with_type_hint(LogDriver *d, const char *name, const char *value, const char *type_hint);
void lua_dd_init_global_contants(LogDriver *d);

LogTemplateOptions *lua_dd_get_template_options(LogDriver *d);

#endif
