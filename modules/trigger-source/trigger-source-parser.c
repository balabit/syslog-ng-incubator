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

#include "trigger-source.h"
#include "cfg-parser.h"
#include "trigger-source-grammar.h"

extern int trigger_source_debug;
int trigger_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword trigger_keywords[] = {
  { "trigger",                 KW_TRIGGER },
  { "trigger_freq",            KW_TRIGGER_FREQ },
  { "trigger_message",         KW_TRIGGER_MESSAGE },
  { NULL }
};

CfgParser trigger_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &trigger_source_debug,
#endif
  .name = "trigger-source",
  .keywords = trigger_keywords,
  .parse = (int (*)(CfgLexer *lexer, gpointer *instance, gpointer)) trigger_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(trigger_, LogDriver **)
