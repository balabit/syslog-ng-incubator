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

#include "lua-template.h"
#include "lua-utils.h"
#include <syslog-ng.h>
#include "scratch-buffers.h"
#include "lua-msg.h"
#include <lauxlib.h>
#include <lualib.h>

#define LUA_TEMPLATE_TYPE "SyslogNG.Template"

#define LUA_COMPAT_MODULE

int
lua_template_new(lua_State *state)
{
  GlobalConfig *cfg;
  LogTemplate *template;
  const char *template_string;
  GError *error = NULL;

  cfg = lua_get_config_from_current_state(state);
  template_string = lua_tostring(state, -1);

  template = log_template_new(cfg, NULL);
  log_template_compile(template, template_string, &error);
  if (error != NULL)
  {
     return luaL_error(state, "%s", error->message);
  }
  return lua_create_userdata_from_pointer(state, template, LUA_TEMPLATE_TYPE);
};

static int
lua_template_metatable__gc(lua_State *state)
{
  LogTemplate *template;

  template = (LogTemplate *) lua_check_and_convert_userdata(state, -1, LUA_TEMPLATE_TYPE);
  log_template_unref(template);

  return 0;
}

static int
lua_template_format(lua_State *state)
{
  LogTemplate *template;
  LogMessage *msg;
  SBGString *str = sb_gstring_acquire();

  template = (LogTemplate *) lua_check_and_convert_userdata(state, -2, LUA_TEMPLATE_TYPE);
  msg = (LogMessage *) lua_check_and_convert_userdata(state, -1, LUA_MESSAGE_TYPE);

  log_template_format(template, msg, NULL, 0, 0, NULL, sb_gstring_string(str));
  lua_pushlstring(state, sb_gstring_string(str)->str, sb_gstring_string(str)->len);

  sb_gstring_release(str);
  return 1;
}

static const struct luaL_Reg template_namespace [] =
{
  {"new", lua_template_new},
  {NULL, NULL}
};

static const struct luaL_Reg template_methods [] =
{
  {"format", lua_template_format},
  {"__gc", lua_template_metatable__gc},
  {NULL, NULL}
};

int
lua_register_template_class(lua_State *state)
{
  luaL_newmetatable(state, LUA_TEMPLATE_TYPE);

  lua_pushstring(state, "__index");
  lua_pushvalue(state, -2);
  lua_settable(state, -3);

  luaL_openlib(state, NULL, template_methods, 0);

  lua_pop(state, 1);
  luaL_openlib(state, "Template", template_namespace, 0);

  return 0;
}

