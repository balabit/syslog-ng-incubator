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
#include "cfg-parser.h"
#include "monitor-source-grammar.h"

extern int monitor_source_debug;
int monitor_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword monitor_keywords[] = {
  { "monitor",                 KW_MONITOR },
  { "monitor_freq",            KW_MONITOR_FREQ },
  { "monitor_script",          KW_MONITOR_SCRIPT },
  { "monitor_func",            KW_MONITOR_FUNC },
  { NULL }
};

CfgParser monitor_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &monitor_source_debug,
#endif
  .name = "monitor-source",
  .keywords = monitor_keywords,
  .parse = (int (*)(CfgLexer *lexer, gpointer *instance, gpointer)) monitor_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(monitor_, LogDriver **)
