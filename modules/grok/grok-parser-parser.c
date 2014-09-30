/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Gergely Nagy <algernon@balabit.hu>
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

#include "grok-parser.h"
#include "cfg-parser.h"
#include "grok-parser-grammar.h"
#include "grok-parser-parser.h"


extern int grok_parser_debug;
int grok_parse(CfgLexer *lexer, LogParser **instance, gpointer arg);

static CfgLexerKeyword grok_keywords[] = {
  { "grok",                 KW_GROK },
  { "match",            KW_GROK_MATCH },
  { "pattern_directory",            KW_GROK_PATTERN_DIRECTORY },
  { "custom_pattern",            KW_GROK_CUSTOM_PATTERN },
  { NULL }
};

CfgParser grok_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &grok_parser_debug,
#endif
  .name = "grok-parser",
  .keywords = grok_keywords,
  .parse = (int (*)(CfgLexer *, gpointer *, gpointer)) grok_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(grok_, LogParser **);

