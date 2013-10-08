/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Gyula Petrovics <pgyula@balabit.hu>
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

#include "mysql-mod.h"
#include "cfg-parser.h"
#include "mysql-mod-grammar.h"

extern int mysql_debug;

int mysql_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword mysql_keywords[] = {
  { "mysql",              KW_MYSQL },
  { "username",           KW_USERNAME },
  { "password",           KW_PASSWORD },
  { "bulk_insert",        KW_BULK_INSERT },
  { "database",           KW_DATABASE },
  { "table",              KW_TABLE },

  { "columns",            KW_COLUMNS },
  { "indexes",            KW_INDEXES },
  { "values",             KW_VALUES },
  { "log_fifo_size",      KW_LOG_FIFO_SIZE },
  { "frac_digits",        KW_FRAC_DIGITS },
  { "session_statements", KW_SESSION_STATEMENTS, 0x0302 },
  { "host",               KW_HOST },
  { "port",               KW_PORT },

  { "time_zone",          KW_TIME_ZONE },
  { "local_time_zone",    KW_LOCAL_TIME_ZONE },
  { "null",               KW_NULL },
  { "retry_sql_inserts",  KW_RETRIES, 0x0303 },
  { "retries",            KW_RETRIES, 0x0303 },
  { "flush_lines",        KW_FLUSH_LINES },
  { "flush_timeout",      KW_FLUSH_TIMEOUT },

  { NULL }
};

CfgParser mysql_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &mysql_debug,
#endif
  .name = "mysql",
  .keywords = mysql_keywords,
  .parse = (gint (*)(CfgLexer *, gpointer *, gpointer)) mysql_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(mysql_, LogDriver **)
