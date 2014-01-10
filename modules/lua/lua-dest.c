/*
 * Copyright (c) 2013, 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013, 2014 Viktor Tusa <tusa@balabit.hu>
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

#include "lua-dest.h"
#include "messages.h"
#include <lauxlib.h>
#include <lualib.h>
#include "scratch-buffers.h"

static gboolean
lua_dd_load_file(LuaDestDriver* self)
{
  if (luaL_loadfile(self->state, self->filename) ||
      lua_pcall(self->state, 0,0,0) )
    {
      msg_error("Error parsing lua script file for lua destination",
                evt_tag_str("error", lua_tostring(self->state, -1)),
                evt_tag_str("filename", self->filename),
                evt_tag_str("driver_id", self->super.super.id),
                NULL);
      return FALSE;
    }
  return TRUE;
};

static void
lua_dd_queue(LogPipe *s, LogMessage *msg, const LogPathOptions *path_options,
             gpointer user_data)
{
  LuaDestDriver *self = (LuaDestDriver *) s;
  SBGString *str = sb_gstring_acquire();

  log_template_format(self->template, msg, NULL, 0, 0, NULL, sb_gstring_string(str));

  lua_getglobal(self->state, self->queue_func_name);
  lua_pushlstring(self->state, sb_gstring_string(str)->str, sb_gstring_string(str)->len);

  if (lua_pcall(self->state, 1, 0, 0))
    msg_error("Error happened during calling Lua destination function!",
              evt_tag_str("error", lua_tostring(self->state, -1)),
              evt_tag_str("queue_func", self->queue_func_name),
              evt_tag_str("filename", self->filename),
              evt_tag_str("driver_id", self->super.super.id),
              NULL);

  log_dest_driver_queue_method(s, msg, path_options, user_data);
  sb_gstring_release(str);
};

static gboolean
lua_dd_call_init_func(LuaDestDriver* self)
{
  msg_debug("Calling lua destination init function", NULL);

  lua_getglobal(self->state, self->init_func_name);

  if (lua_isnil(self->state, -1))
    {
      msg_warning("Lua destination initializing function cannot be found, continueing anyway!",
                  evt_tag_str("init_func", self->init_func_name),
                  evt_tag_str("filename", self->filename),
                  evt_tag_str("driver_id", self->super.super.id),
                  NULL);
      return TRUE;
    }

  if (lua_pcall(self->state, 0, 0, 0))
    {
      msg_error("Error happened during calling Lua destination initializing function!",
                evt_tag_str("error", lua_tostring(self->state, -1)),
                evt_tag_str("init_func", self->init_func_name),
                evt_tag_str("filename", self->filename),
                evt_tag_str("driver_id", self->super.super.id),
                NULL);
      return FALSE;
    }
  return TRUE;
}

static gboolean
lua_dd_check_existence_of_queue_func(LuaDestDriver* self)
{
  gboolean result = TRUE;

  lua_getglobal(self->state, self->queue_func_name);
  if (lua_isnil(self->state, -1))
    {
      msg_error("Lua destination queue function cannot be found!",
                evt_tag_str("queue_func", self->queue_func_name),
                evt_tag_str("filename", self->filename),
                evt_tag_str("driver_id", self->super.super.id),
                NULL);
      result = FALSE;
    }
  lua_pop(self->state, 1);

  return result;
}

static gboolean
lua_dd_init(LogPipe *s)
{
  LuaDestDriver *self = (LuaDestDriver *) s;
  GlobalConfig* cfg;

  if (!lua_dd_load_file(self))
    {
      lua_close(self->state);
      return FALSE;
    }

  cfg = log_pipe_get_config(s);

  if (!self->template)
    {
      msg_info("No template set in lua destination, falling back to template \"$MESSAGE\"",
               evt_tag_str("driver_id", self->super.super.id),
               NULL);
      self->template = log_template_new(cfg, "default_lua_template");
      log_template_compile(self->template, "$MESSAGE", NULL);
    }

  if (!self->init_func_name)
    {
      msg_info("No init function name set, defaulting to \"lua_init_func\"",
               evt_tag_str("driver_id", self->super.super.id),
               NULL);
      self->init_func_name = g_strdup("lua_init_func");
  }

  if (!self->queue_func_name)
    {
      msg_info("No queue function name set, defaulting to \"lua_queue_func\"",
               evt_tag_str("driver_id", self->super.super.id),
               NULL);
      self->queue_func_name = g_strdup("lua_queue_func");
    }

  if (!lua_dd_call_init_func(self))
    {
      return FALSE;
    }

  if (!lua_dd_check_existence_of_queue_func(self))
    {
      return FALSE;
    }

  return log_dest_driver_init_method(s);
}

static gboolean
lua_dd_deinit(LogPipe *s)
{
  LuaDestDriver *self = (LuaDestDriver *) s;

  if (!log_dest_driver_deinit_method(s))
    return FALSE;

  lua_close(self->state);
  return TRUE;
}

static void
lua_dd_free(LogPipe *s)
{
  LuaDestDriver *self = (LuaDestDriver *) s;

  log_dest_driver_free(s);
  log_template_unref(self->template);
  g_free(self->filename);
  g_free(self->init_func_name);
  g_free(self->queue_func_name);
}

void
lua_dd_set_template(LogDriver* d, LogTemplate* template)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  self->template = log_template_ref(template);
}

void 
lua_dd_set_filename(LogDriver* d, gchar* filename)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  self->filename = g_strdup(filename);
}

LogTemplateOptions *
lua_dd_get_template_options(LogDriver *d)
{
  LuaDestDriver *self = (LuaDestDriver *)d;

  return &self->template_options;
}

void
lua_dd_set_init_func(LogDriver* d, gchar* init_func_name)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  g_free(self->init_func_name);
  self->init_func_name = g_strdup(init_func_name);
}

void
lua_dd_set_queue_func(LogDriver* d, gchar* queue_func_name)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  g_free(self->queue_func_name);
  self->queue_func_name = g_strdup(queue_func_name);
}

LogDriver*
lua_dd_new()
{
  LuaDestDriver *self = g_new0(LuaDestDriver, 1);

  self->state = luaL_newstate();
  luaL_openlibs(self->state);

  log_dest_driver_init_instance(&self->super);
  self->super.super.super.init = lua_dd_init;
  self->super.super.super.deinit = lua_dd_deinit;
  self->super.super.super.free_fn = lua_dd_free;
  self->super.super.super.queue = lua_dd_queue;

  return &self->super.super;
}
