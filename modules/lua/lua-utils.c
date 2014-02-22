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

#include "lua-utils.h"
#include <lauxlib.h>
#include "messages.h"

static void *
lua_get_pointer_from_userdata(void *udata)
{
  void *data;

  if (!udata)
    return NULL;

  data = *((void **)udata);

  return data;
}

static void *
lua_check_user_data (lua_State *state, int userdata_index, const char *type)
{
  void *data = lua_touserdata(state, userdata_index);

  if (!data)
    return NULL;

  if (lua_getmetatable(state, userdata_index))
    {
      lua_getfield(state, LUA_REGISTRYINDEX, type);

      if (lua_rawequal(state, -1, -2))
        {
          lua_pop(state, 2);
          return data;
        }

      lua_pop(state, 2);

    }
  return NULL;
}

void *
lua_check_and_convert_userdata(lua_State *state, int index, const char *type)
{
  return lua_get_pointer_from_userdata(lua_check_user_data(state, index, type));
}

int
lua_create_userdata_from_pointer(lua_State *state, void *data, const char *type)
{
  void *userdata = lua_newuserdata(state, sizeof(data));

  *((void **)userdata) = data;

  luaL_getmetatable(state, type);
  lua_setmetatable(state, -2);
  return 1;
}

gboolean
lua_check_existence_of_global_variable(lua_State *state, const char *variable_name)
{
  gboolean result = TRUE;

  lua_getglobal(state, variable_name);
  if (lua_isnil(state, -1))
    {
      result = FALSE;
    }
  lua_pop(state, 1);

  return result;
};

GlobalConfig *
lua_get_config_from_current_state(lua_State *state)
{
    GlobalConfig *result;

    lua_getglobal(state, "__conf");
    result = lua_touserdata(state, -1);
    lua_pop(state, 1);
    return result;
}

static int
lua_msg_debug(lua_State *state)
{
  const char *message = lua_tostring(state, -1);
  msg_debug(message, NULL);
  return 0;
};

static int
lua_msg_error(lua_State *state)
{
  const char *message = lua_tostring(state, -1);
  msg_error(message, NULL);
  return 0;
};

static int
lua_msg_info(lua_State *state)
{
  const char *message = lua_tostring(state, -1);
  msg_info(message, NULL);
  return 0;
};

static int
lua_msg_verbose(lua_State *state)
{
  const char *message = lua_tostring(state, -1);
  msg_verbose(message, NULL);
  return 0;
};

static const struct luaL_Reg utility_functions [] =
{
  {"debug", lua_msg_debug},
  {"error", lua_msg_error},
  {"verbose", lua_msg_verbose},
  {"info", lua_msg_info},

  {NULL, NULL}
};

void lua_register_utility_functions(lua_State *state)
{
  luaL_openlib(state, "syslogng", utility_functions, 0);
};
