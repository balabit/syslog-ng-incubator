/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Viktor Tusa <tusa@balabit.hu>
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
#include <lua.h>

typedef struct _LuaDestDriver
{
  LogDestDriver super;
  lua_State* state;
  gchar* template_string;
  gchar* filename;
  gchar* init_func_name;
  gchar* queue_func_name;
  LogTemplate* template;
  LogTemplateOptions template_options;
} LuaDestDriver;

LogDriver* lua_dd_new();
void lua_dd_set_init_func(LogDriver* d, gchar* init_func_name);
void lua_dd_set_queue_func(LogDriver* d, gchar* queue_func_name);
LogTemplateOptions* lua_dd_get_template_options(LogDriver *d);
#endif
