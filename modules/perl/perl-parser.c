/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
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

#include "perl-parser.h"
#include "cfg-parser.h"
#include "perl-grammar.h"

extern int perl_debug;
int perl_driver_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword perl_keywords[] = {
  { "perl",                     KW_PERL },
  { "script",                   KW_SCRIPT },
  { "init_func",                KW_INIT_FUNC },
  { "queue_func",               KW_QUEUE_FUNC },
  { "deinit_func",              KW_DEINIT_FUNC },
  { NULL }
};

CfgParser perl_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &perl_debug,
#endif
  .name = "perl",
  .keywords = perl_keywords,
  .parse = (int (*)(CfgLexer *lexer, gpointer *instance, gpointer)) perl_driver_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(perl_driver_, LogDriver **)
