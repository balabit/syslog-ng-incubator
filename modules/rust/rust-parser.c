/*
 * Copyright (c) 2002-2014 BalaBit S.a.r.l.
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

#include "cfg-parser.h"
#include "filter/filter-expr.h"
#include "rust-grammar.h"

extern int rust_debug;

int rust_parse(CfgLexer *lexer, gpointer *instance, gpointer arg);

static CfgLexerKeyword rust_keywords[] = {
  { "rust",     KW_RUST },
  { "option",   KW_OPTION },
  { "type",     KW_TYPE },
  { NULL }
};
 
CfgParser rust_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &rust_debug,
#endif
  .name = "rust",
  .keywords = rust_keywords,
  .parse = (gint (*)(CfgLexer *, gpointer *, gpointer)) rust_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(rust_, gpointer *)

