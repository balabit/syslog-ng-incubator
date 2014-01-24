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
