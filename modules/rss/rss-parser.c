/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Tusa Viktor
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include "rss.h"
#include "cfg-parser.h"
#include "rss-grammar.h"

extern int rss_debug;
int rss_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword rss_keywords[] = {
  { "rss",                    KW_RSS },
  { "port",                   KW_RSS_PORT },
  { "feed_title",             KW_RSS_TITLE },
  { "entry_title",            KW_RSS_ENTRY_TITLE },
  { "entry_description",      KW_RSS_ENTRY_DESCRIPTION },
  { NULL }
};

CfgParser rss_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &rss_debug,
#endif
  .name = "rss",
  .keywords = rss_keywords,
  .parse = (int (*)(CfgLexer *lexer, gpointer *instance, gpointer arg)) rss_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(rss_, LogDriver **)
