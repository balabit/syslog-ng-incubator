/*
 * Copyright (c) 2014 Pierre-Yves Ritschard <pyr@spootnik.org>
 *
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

#include "kafka.h"
#include "cfg-parser.h"
#include "kafka-grammar.h"

extern int kafka_debug;
int kafka_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword kafka_keywords[] = {
    { "field",          KW_FIELD },
    { "kafka",          KW_KAFKA },
    { "partition",      KW_PARTITION },
    { "payload",        KW_PAYLOAD },
    { "properties",     KW_PROP },
    { "random",         KW_RANDOM },
    { "topic",			KW_TOPIC },
    { NULL }
};

CfgParser kafka_parser =
{
#if ENABLE_DEBUG
  .debug_flag = &kafka_debug,
#endif
  .name = "kafka",
  .keywords = kafka_keywords,
  .parse = (int (*)(CfgLexer *lexer, gpointer *instance, gpointer)) kafka_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(kafka_, LogDriver **)
