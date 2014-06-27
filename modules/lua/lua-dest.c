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

#include "lua-dest.h"
#include "lua-msg.h"
#include "lua-template.h"
#include "lua-utils.h"
#include "messages.h"
#include <lauxlib.h>
#include <lualib.h>
#include "scratch-buffers.h"

#define LUA_DEST_MODE_RAW 1
#define LUA_DEST_MODE_FORMATTED 2

#ifndef SCS_LUA
#define SCS_LUA 0
#endif

typedef struct _LuaGlobalConstant {
   char *name;
   char *value;
   TypeHint type;
} LuaGlobalConstant;

static void
lua_global_constant_free(LuaGlobalConstant *self)
{
  g_free(self->name);
  g_free(self->value);
  g_free(self);
};

static gboolean
lua_dd_load_file(LuaDestDriver *self)
{
  if (luaL_loadfile(self->state, self->filename) ||
      lua_pcall(self->state, 0,0,0) )
    {
      msg_error("Error parsing lua script file for lua destination",
                evt_tag_str("error", lua_tostring(self->state, -1)),
                evt_tag_str("filename", self->filename),
                evt_tag_str("driver_id", self->super.super.super.id),
                NULL);
      return FALSE;
    }
  return TRUE;
};

static void lua_dd_accept_message(LogMessage *msg, LogPathOptions *path_options)
{
   log_msg_ack(msg, path_options);
   log_msg_unref(msg);
};

static void lua_dd_ack_message(LuaDestDriver *self, LogMessage *msg, LogPathOptions *path_options)
{
   stats_counter_inc(self->super.stored_messages);
   lua_dd_accept_message(msg, path_options);
};

static void lua_dd_drop_message(LuaDestDriver *self, LogMessage *msg, LogPathOptions *path_options)
{
   stats_counter_inc(self->super.dropped_messages);
   lua_dd_accept_message(msg, path_options);
};

static void 
lua_cast_and_push_value_to_stack(lua_State *state, const gchar *name, TypeHint type, const gchar *value)
{
  switch (type)
    {
    case TYPE_HINT_INT32:
      {
        gint32 i;

        if (type_cast_to_int32(value, &i, NULL))
          lua_pushinteger(state, i);
        else
          msg_error("Cannot cast value to integer",
                    evt_tag_str("name", name),
                    evt_tag_str("value", value),
                    NULL);

        break;
      }
    case TYPE_HINT_STRING:
      lua_pushstring(state, value);
      break;
    default:
      msg_error("Unsupported type hint (strings or integers only!)",
                evt_tag_str("name", name),
                evt_tag_str("value", value),
                NULL);
      break;
    }

};

static gboolean
lua_dd_add_parameter_to_table(const gchar *name, TypeHint type, const gchar *value, gpointer user_data)
{
  lua_State *state = (lua_State *) user_data;

  lua_pushstring(state, name);
  lua_cast_and_push_value_to_stack(state, name, type, value);
  
  lua_settable(state, -3);
  return FALSE;
};

static void
lua_dd_create_parameter_table_for_queue_func(lua_State *state, ValuePairs *params, LogMessage *msg)
{
  lua_newtable(state);
  value_pairs_foreach(params, lua_dd_add_parameter_to_table, msg, 0,
                      NULL, state);
}

static gboolean
lua_dd_queue(LogThrDestDriver *d)
{
  LuaDestDriver *self = (LuaDestDriver *) d;
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;
  LogMessage *msg;
  SBGString *str = sb_gstring_acquire();
  gboolean success;
  int number_of_parameters = 1;

  success = log_queue_pop_head(self->super.queue, &msg, &path_options, FALSE, FALSE);
  if (!success)
    return TRUE;

  msg_set_context(msg);

  lua_getglobal(self->state, self->queue_func_name);

  if (self->mode == LUA_DEST_MODE_FORMATTED)
    {
      log_template_format(self->template, msg, NULL, 0, 0, NULL, sb_gstring_string(str));
      lua_pushlstring(self->state, sb_gstring_string(str)->str, sb_gstring_string(str)->len);
    }

  if (self->mode == LUA_DEST_MODE_RAW)
    {
      lua_message_create_from_logmsg(self->state, msg);
    }

  if (self->params)
    {
      lua_dd_create_parameter_table_for_queue_func(self->state, self->params, msg);
      number_of_parameters = 2;
    }

  success = !lua_pcall(self->state, number_of_parameters, 0, 0);

  msg_set_context(NULL);
  sb_gstring_release(str);

  if (success)
   {
     lua_dd_ack_message(self, msg, &path_options);
   }
  else
   {
     msg_error("Error happened during calling Lua destination function!",
              evt_tag_str("error", lua_tostring(self->state, -1)),
              evt_tag_str("queue_func", self->queue_func_name),
              evt_tag_str("filename", self->filename),
              evt_tag_str("driver_id", self->super.super.super.id),
              NULL);

     lua_dd_drop_message(self, msg, &path_options);
   }  

  return success;
};


static gboolean
lua_dd_check_and_call_function(LuaDestDriver *self, const char *function_name, const char *function_type)
{
  msg_debug("Calling lua destination function", evt_tag_str("function_type", function_type), NULL);

  lua_getglobal(self->state, function_name);

  if (lua_isnil(self->state, -1))
    {
      msg_warning("Lua destination function cannot be found, continueing anyway!",
                  evt_tag_str("function_type", function_type),
                  evt_tag_str("function_name", function_name),
                  evt_tag_str("filename", self->filename),
                  evt_tag_str("driver_id", self->super.super.super.id),
                  NULL);
      return TRUE;
    }

  if (lua_pcall(self->state, 0, 0, 0))
    {
      msg_error("Error happened during calling Lua destination initializing function!",
                evt_tag_str("error", lua_tostring(self->state, -1)),
                evt_tag_str("function_type", function_type),
                evt_tag_str("function_name", function_name),
                evt_tag_str("filename", self->filename),
                evt_tag_str("driver_id", self->super.super.super.id),
                NULL);
      return FALSE;
    }
  return TRUE;
};

static gboolean
lua_dd_call_init_func(LuaDestDriver *self)
{
  return lua_dd_check_and_call_function(self, self->init_func_name, "initialization");
}

static gboolean
lua_dd_call_deinit_func(LuaDestDriver *self)
{
  return lua_dd_check_and_call_function(self, self->deinit_func_name, "deinitialization");
}

static gboolean
lua_dd_check_existence_of_queue_func(LuaDestDriver *self)
{
  if (!lua_check_existence_of_global_variable(self->state, self->queue_func_name))
    {
      msg_error("Lua destination queue function cannot be found!",
                evt_tag_str("queue_func", self->queue_func_name),
                evt_tag_str("filename", self->filename),
                evt_tag_str("driver_id", self->super.super.super.id),
                NULL);
      return FALSE;
    }

  return TRUE;
}

static void
lua_dd_set_config_variable(lua_State *state, GlobalConfig *conf)
{
  lua_pushlightuserdata(state, conf);
  lua_setglobal(state, "__conf");
};

static gboolean
lua_dd_inject_global_variable(gpointer data,
                              gpointer user_data)
{
  LuaGlobalConstant *constant = (LuaGlobalConstant *) data;
  lua_State *state = (lua_State *)user_data;

  lua_cast_and_push_value_to_stack(state, constant->name, constant->type, constant->value);

  lua_setglobal(state, constant->name);

  return FALSE;
}

static void
lua_dd_inject_all_global_variables(lua_State *state, GList *globals)
{
  g_list_foreach(globals, lua_dd_inject_global_variable, state);
};

static gboolean
lua_dd_init(LogPipe *s)
{
  LuaDestDriver *self = (LuaDestDriver *) s;
  GlobalConfig *cfg;

  if (!lua_dd_load_file(self))
    {
      lua_close(self->state);
      return FALSE;
    }

  lua_register_message(self->state);
  lua_register_template_class(self->state);

  lua_register_utility_functions(self->state);

  cfg = log_pipe_get_config(s);

  lua_dd_set_config_variable(self->state, cfg);
  if (self->globals)
    {
      lua_dd_inject_all_global_variables(self->state, self->globals);
      g_list_free_full(self->globals, lua_global_constant_free);
      self->globals = NULL;
    }

  if (!self->template)
    {
      msg_info("No template set in lua destination, falling back to template \"$MESSAGE\"",
               evt_tag_str("driver_id", self->super.super.super.id),
               NULL);
      self->template = log_template_new(cfg, "default_lua_template");
      log_template_compile(self->template, "$MESSAGE", NULL);
    }

  if (!self->init_func_name)
    {
      msg_info("No init function name set, defaulting to \"lua_init_func\"",
               evt_tag_str("driver_id", self->super.super.super.id),
               NULL);
      self->init_func_name = g_strdup("lua_init_func");
    }

  if (!self->queue_func_name)
    {
      msg_info("No queue function name set, defaulting to \"lua_queue_func\"",
               evt_tag_str("driver_id", self->super.super.super.id),
               NULL);
      self->queue_func_name = g_strdup("lua_queue_func");
    }

  if (!self->deinit_func_name)
    {
      msg_info("No deinit function name set, defaulting to \"lua_deinit_func\"",
               evt_tag_str("driver_id", self->super.super.super.id),
               NULL);
      self->deinit_func_name = g_strdup("lua_deinit_func");
    }

  if (!self->mode)
    self->mode = LUA_DEST_MODE_FORMATTED;

  if (!lua_dd_call_init_func(self))
    {
      return FALSE;
    }

  if (!lua_dd_check_existence_of_queue_func(self))
    {
      return FALSE;
    }

  return log_threaded_dest_driver_start(s);
}

static gboolean
lua_dd_deinit(LogPipe *s)
{
  LuaDestDriver *self = (LuaDestDriver *) s;

  lua_dd_call_deinit_func(self);

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
lua_dd_set_template(LogDriver *d, LogTemplate *template)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  self->template = log_template_ref(template);
}

void
lua_dd_set_filename(LogDriver *d, gchar *filename)
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
lua_dd_set_init_func(LogDriver *d, gchar *init_func_name)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  g_free(self->init_func_name);
  self->init_func_name = g_strdup(init_func_name);
}

void
lua_dd_set_queue_func(LogDriver *d, gchar *queue_func_name)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  g_free(self->queue_func_name);
  self->queue_func_name = g_strdup(queue_func_name);
}

void
lua_dd_set_deinit_func(LogDriver *d, gchar *deinit_func_name)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  g_free(self->deinit_func_name);
  self->deinit_func_name = g_strdup(deinit_func_name);
}

void
lua_dd_set_mode(LogDriver *d, gchar *mode)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  if (!strcmp("raw", mode))
    self->mode = LUA_DEST_MODE_RAW;

  if (!strcmp("formatted", mode))
    self->mode = LUA_DEST_MODE_FORMATTED;
};

void
lua_dd_init_global_contants(LogDriver *d)
{
  LuaDestDriver *self = (LuaDestDriver *)d;
  
  if (self->globals)
    g_list_free_full(self->globals, lua_global_constant_free);

  self->globals = NULL;
};

static void
lua_dd_create_global_constant_from_params(LuaDestDriver *self, const char *name, const char *value, TypeHint type)
{
  LuaGlobalConstant *constant = g_new0(LuaGlobalConstant, 1);
  constant->name = g_strdup(name);
  constant->value = g_strdup(value);
  constant->type = type;

  self->globals = g_list_append(self->globals, constant); 
};

void
lua_dd_add_global_constant(LogDriver *d, const char *name, const char *value)
{
  LuaDestDriver *self = (LuaDestDriver *)d;
  lua_dd_create_global_constant_from_params(self, name, value, TYPE_HINT_STRING);
};

void
lua_dd_add_global_constant_with_type_hint(LogDriver *d, const char *name, const char *value, const char *type)
{
  TypeHint type_hint;
  LuaDestDriver *self = (LuaDestDriver *)d;
  
  if (!type_hint_parse(type, &type_hint, NULL))
   {
     msg_error("Type hint parsing failed for lua constant, adding with default type hint",
	       evt_tag_str("name", name),
	       evt_tag_str("value", value),
	       evt_tag_str("type", type),
               NULL);
     lua_dd_create_global_constant_from_params(self, name, value, TYPE_HINT_STRING);
   }
  else
   {
     lua_dd_create_global_constant_from_params(self, name, value, type_hint);
   }
  
};

static gchar *
lua_dd_format_string_instance(LogThrDestDriver *d, char* name, size_t name_len)
{
  LuaDestDriver *self = (LuaDestDriver *)d;

  g_snprintf(name, name_len,
             "python,%s,%s,%s,%s",
             self->filename,
             self->init_func_name,
             self->queue_func_name,
             self->deinit_func_name);
  return name;
};

static gchar *
lua_dd_format_stats_instance(LogThrDestDriver *d)
{
  static gchar stats_name[1024];

  return lua_dd_format_string_instance(d, stats_name, sizeof(stats_name));
}

static gchar *
lua_dd_format_persist_name(LogThrDestDriver *d)
{
  static gchar persist_name[1024];

  return lua_dd_format_string_instance(d, persist_name, sizeof(persist_name));
}

void
lua_dd_set_params(LogDriver *d, ValuePairs *vp)
{
  LuaDestDriver *self = (LuaDestDriver *) d;

  if (self->params)
    value_pairs_free(self->params);
  self->params = vp;
}

LogDriver *
lua_dd_new()
{
  LuaDestDriver *self = g_new0(LuaDestDriver, 1);

  self->state = luaL_newstate();
  luaL_openlibs(self->state);

  log_threaded_dest_driver_init_instance(&self->super);
  self->super.super.super.super.init = lua_dd_init;
  self->super.super.super.super.deinit = lua_dd_deinit;
  self->super.super.super.super.free_fn = lua_dd_free;

  self->super.worker.insert = lua_dd_queue;
  self->super.worker.disconnect = NULL;

  self->super.format.stats_instance = lua_dd_format_stats_instance;
  self->super.format.persist_name = lua_dd_format_persist_name;
  self->super.stats_source = SCS_LUA;

  return &self->super.super.super;
}
