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

#define LUA_COMPAT_MODULE

#include "lua-msg.h"
#include "lua-utils.h"
#include "logmsg.h"
#include "messages.h"
#include <lauxlib.h>
#include <lualib.h>

int
lua_message_create_from_logmsg(lua_State *state, LogMessage *self)
{
  log_msg_ref(self);
  return lua_create_userdata_from_pointer(state, self, LUA_MESSAGE_TYPE);
};

static int
lua_message_new(lua_State *state)
{
  LogMessage *self = log_msg_new_empty();

  self->pri = 0;
  self->flags |= LF_LOCAL;

  return lua_message_create_from_logmsg(state, self);
}

static int
lua_message_metatable__new_index(lua_State *state)
{
  gsize value_len = 0;
  LogMessage *m = lua_check_and_convert_userdata(state, 1, LUA_MESSAGE_TYPE);

  const char *key = lua_tostring(state, 2);
  const char *value = lua_tolstring(state, 3, &value_len);

  NVHandle nv_handle = log_msg_get_value_handle(key);

  msg_trace("Setting value to lua message",
            evt_tag_str("key",key),
            evt_tag_str("value",value),
            NULL);
  log_msg_set_value(m, nv_handle, value, value_len);
  return 0;
}

static int
lua_message_metatable__index(lua_State *state)
{
  gssize value_len;
  LogMessage *m = lua_check_and_convert_userdata(state, 1, LUA_MESSAGE_TYPE);
  const char *key = lua_tostring(state, 2);

  NVHandle handle = log_msg_get_value_handle(key);
  const char *value = log_msg_get_value(m, handle, &value_len);

  msg_trace("Getting value from lua message",
            evt_tag_str("key",key),
            evt_tag_str("value",value),
            NULL);
  lua_pushlstring(state, value, value_len);

  return 1;
}

static int
lua_message_metatable__gc(lua_State *state)
{
  LogMessage *m = lua_check_and_convert_userdata(state, 1, LUA_MESSAGE_TYPE);

  log_msg_unref(m);

  return 0;
};

LogMessage *
lua_message_to_logmsg(lua_State *state, int index)
{
  return lua_check_and_convert_userdata(state, index, LUA_MESSAGE_TYPE);
}

static const struct luaL_Reg msg_function [] =
{
  {"new", lua_message_new},
  {NULL, NULL}
};

int
lua_register_message(lua_State *state)
{
  luaL_newmetatable(state, LUA_MESSAGE_TYPE);

  lua_pushstring(state, "__newindex");
  lua_pushcfunction(state, lua_message_metatable__new_index);
  lua_settable(state, -3);

  lua_pushstring(state, "__index");
  lua_pushcfunction(state, lua_message_metatable__index);
  lua_settable(state, -3);

  lua_pushstring(state, "__gc");
  lua_pushcfunction(state, lua_message_metatable__gc);
  lua_settable(state, -3);

  lua_pop(state, 1);
  luaL_openlib(state, "Message", msg_function, 0);

  return 0;
}
