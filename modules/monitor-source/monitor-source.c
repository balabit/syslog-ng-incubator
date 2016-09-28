/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Gergely Nagy <algernon@balabit.hu>
 * Copyright (c) 2014 Viktor Tusa <tusa@balabit.hu>
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

#include "monitor-source.h"
#include "../lua/lua-utils.h"
#include <lualib.h>
#include <lauxlib.h>

#include "driver.h"
#include "logsource.h"
#include "mainloop.h"

#include <iv.h>

#define LUA_COMPAT_MODULE

typedef struct
{
  gint monitor_freq;
  gchar *message;
  gchar *script;
  gchar *monitor_func_name;
} MonitorOptions;

typedef struct _MonitorSourceDriver
{
  LogSrcDriver super;
  LogSource *source;
  LogSourceOptions source_options;

  MonitorOptions options;
} MonitorSourceDriver;

typedef struct
{
  LogSource super;
  lua_State *state;
  struct iv_timer monitor_timer;
  gboolean watches_running;

  MonitorOptions *options;
} MonitorSource;

#ifndef SCS_MONITOR
#define SCS_MONITOR 0
#endif

/*
 * MonitorSource
 */

static void monitor_source_update_watches (MonitorSource *self);

static gboolean
monitor_source_load_file(MonitorSource *self)
{
  if (luaL_loadfile(self->state, self->options->script) ||
      lua_pcall(self->state, 0,0,0) )
    {
      msg_error("Error parsing lua script file for lua destination",
                evt_tag_str("error", lua_tostring(self->state, -1)),
                evt_tag_str("filename", self->options->script),
                NULL);
      return FALSE;
    }
  return TRUE;
};

static void
monitor_process_string_result(lua_State *state, LogMessage *msg, const char *key)
{
  const gchar *value;
  size_t value_length;
  NVHandle handle;

  value = lua_tolstring(state, -1, &value_length);
  value = g_strndup(value, value_length);
  handle = log_msg_get_value_handle(key);
  log_msg_set_value(msg, handle, value, value_length);
};

static void
monitor_process_result(lua_State *state, LogMessage *msg)
{
  if (lua_isstring(state, -1))
    {
      monitor_process_string_result(state, msg, "MESSAGE");
    }
  else if (lua_istable(state, -1))
    {
      lua_pushnil(state);
      while(lua_next(state, -2))
        {
          const char *key = lua_tostring(state, -2);

          if (lua_isstring(state, -1))
            {
              monitor_process_string_result(state, msg, key);
            }
          lua_pop(state, 1);
        }
    }
  lua_pop(state, 1); 
};

static void
monitor_source_monitored (gpointer s)
{
  MonitorSource *self = (MonitorSource *) s;
  LogMessage *msg;
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;

  main_loop_assert_main_thread ();

  if (!log_source_free_to_send (&self->super))
    goto end;

  msg = log_msg_new_internal (LOG_INFO, "");

  lua_getglobal(self->state, self->options->monitor_func_name);
  if (lua_pcall(self->state, 0, 1, 0))
    {
      msg_error("Error happened during calling monitor source function!",
                evt_tag_str("error", lua_tostring(self->state, -1)),
                evt_tag_str("queue_func", self->options->monitor_func_name),
                evt_tag_str("filename", self->options->script),
                NULL);
      goto end;
    }
  monitor_process_result(self->state, msg);
  path_options.ack_needed = FALSE;

  log_pipe_queue (&self->super.super, msg, &path_options);

 end:
  monitor_source_update_watches (self);
}

static void
monitor_source_init_watches (MonitorSource *self)
{
  IV_TIMER_INIT (&self->monitor_timer);
  self->monitor_timer.cookie = self;
  self->monitor_timer.handler = monitor_source_monitored;
}

static void
monitor_source_start_watches (MonitorSource *self)
{
  if (self->watches_running)
    return;

  if (self->monitor_timer.expires.tv_sec >= 0)
    iv_timer_register (&self->monitor_timer);
  self->watches_running = TRUE;
}

static void
monitor_source_stop_watches (MonitorSource *self)
{
  if (!self->watches_running)
    return;

  if (iv_timer_registered (&self->monitor_timer))
    iv_timer_unregister (&self->monitor_timer);

  self->watches_running = FALSE;
}

static void
monitor_source_update_watches (MonitorSource *self)
{
  if (!log_source_free_to_send (&self->super))
    {
      monitor_source_stop_watches (self);
      return;
    }

  iv_validate_now ();
  monitor_source_stop_watches (self);
  self->monitor_timer.expires = iv_now;
  self->monitor_timer.expires.tv_sec += self->options->monitor_freq;
  monitor_source_start_watches (self);
}

static gboolean
monitor_source_init (LogPipe *s)
{
  MonitorSource *self = (MonitorSource *)s;

  if (!log_source_init (s))
    return FALSE;

  if (!monitor_source_load_file(self))
    {
      return FALSE;
    }

  lua_register_utility_functions(self->state);

  if (!lua_check_existence_of_global_variable(self->state, self->options->monitor_func_name))
    {
      msg_error("Monitor function for monitor source cannot be found!", 
                evt_tag_str("monitor_func", self->options->monitor_func_name),
                evt_tag_str("filename", self->options->script),
                NULL);
      return FALSE;
    }

  iv_validate_now ();
  self->monitor_timer.expires = iv_now;
  self->monitor_timer.expires.tv_sec += self->options->monitor_freq;

  monitor_source_start_watches (self);

  return TRUE;
}

static gboolean
monitor_source_deinit (LogPipe *s)
{
  MonitorSource *self = (MonitorSource *)s;

  monitor_source_stop_watches (self);
  lua_close(self->state);

  return log_source_deinit (s);
}


static LogSource *
monitor_source_new (MonitorSourceDriver *owner, LogSourceOptions *options, GlobalConfig *cfg)
{
  MonitorSource *self = g_new0 (MonitorSource, 1);

  log_source_init_instance (&self->super, cfg);
  log_source_set_options (&self->super, options, 0, SCS_MONITOR,
                          owner->super.super.id, NULL, FALSE, FALSE,
                          owner->super.super.super.expr_node);

  self->state = luaL_newstate();
  luaL_openlibs(self->state);

  self->options = &owner->options;

  monitor_source_init_watches (self);

  self->super.super.init = monitor_source_init;
  self->super.super.deinit = monitor_source_deinit;

  return &self->super;
}

/*
 * MonitorSourceDriver
 */

static gboolean
monitor_sd_init (LogPipe *s)
{
  MonitorSourceDriver *self = (MonitorSourceDriver *)s;
  GlobalConfig *cfg = log_pipe_get_config (s);

  if (!log_src_driver_init_method (s))
    return FALSE;

  if (self->options.monitor_freq <= 0)
    self->options.monitor_freq = 10;

  if (!self->options.script)
    self->options.script = g_strdup("monitor.lua");

  if (!self->options.monitor_func_name)
    self->options.monitor_func_name = g_strdup("monitor");

  log_source_options_init (&self->source_options, cfg, self->super.super.group);
  self->source = monitor_source_new (self, &self->source_options, cfg);

  log_pipe_append (&self->source->super, s);
  log_pipe_init (&self->source->super);

  return TRUE;
}

static gboolean
monitor_sd_deinit (LogPipe *s)
{
  MonitorSourceDriver *self = (MonitorSourceDriver *)s;

  if (self->source)
    {
      log_pipe_deinit (&self->source->super);
      log_pipe_unref (&self->source->super);
      self->source = NULL;
    }

  g_free (self->options.script);
  g_free (self->options.monitor_func_name);

  return log_src_driver_deinit_method (s);
}

LogSourceOptions *
monitor_sd_get_source_options (LogDriver *s)
{
  MonitorSourceDriver *self = (MonitorSourceDriver *)s;

  return &self->source_options;
}

void
monitor_sd_set_monitor_freq (LogDriver *s, gint freq)
{
  MonitorSourceDriver *self = (MonitorSourceDriver *)s;

  self->options.monitor_freq = freq;
}

void
monitor_sd_set_monitor_script (LogDriver *s, const gchar *script)
{
  MonitorSourceDriver *self = (MonitorSourceDriver *)s;

  g_free (self->options.script);
  self->options.script = g_strdup (script);
}

void
monitor_sd_set_monitor_func (LogDriver *s, const gchar *function_name)
{
  MonitorSourceDriver *self = (MonitorSourceDriver *)s;

  g_free (self->options.monitor_func_name);
  self->options.monitor_func_name = g_strdup (function_name);
}

LogDriver *
monitor_sd_new (GlobalConfig *cfg)
{
  MonitorSourceDriver *self = g_new0 (MonitorSourceDriver, 1);

  log_src_driver_init_instance ((LogSrcDriver *)&self->super, cfg);

  self->super.super.super.init = monitor_sd_init;
  self->super.super.super.deinit = monitor_sd_deinit;

  log_source_options_defaults (&self->source_options);

  return (LogDriver *)self;
}
