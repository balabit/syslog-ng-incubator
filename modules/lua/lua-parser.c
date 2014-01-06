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

#include "lua.h"
#include "cfg-parser.h"
#include "lua-grammar.h"

extern int lua_debug;
int lua_driver_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword lua_keywords[] = {
  { "lua",                      KW_LUA },
  { "script",                   KW_SCRIPT },
  { "init",                     KW_INIT },
  { "queue",                    KW_QUEUE },

  { NULL }
};

CfgParser lua_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &lua_debug,
#endif
  .name = "lua",
  .keywords = lua_keywords,
  .parse = (int (*)(CfgLexer *lexer, gpointer *instance, gpointer)) lua_driver_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(lua_driver_, LogDriver **)
