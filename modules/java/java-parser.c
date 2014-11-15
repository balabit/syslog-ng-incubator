/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Tihamer Petrovics <tihameri@gmail.com>
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
#include "java-grammar.h"

int java_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword java_keywords[] = {
  { "java",        KW_JAVA },
  { "class_path",  KW_CLASS_PATH},
  { "class_name",  KW_CLASS_NAME},
  { "option",      KW_OPTION},
  { NULL }
};

CfgParser java_parser =
  {
    .name = "java",
    .keywords = java_keywords,
    .parse = (int (*)(CfgLexer *lexer, gpointer *instance, gpointer)) java_parse,
    .cleanup = (void (*)(gpointer)) log_pipe_unref,
  };

CFG_PARSER_IMPLEMENT_LEXER_BINDING(java_, LogDriver **)
