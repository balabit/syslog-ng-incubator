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

#ifndef SNG_TRIGGER_SOURCE_H_INCLUDED
#define SNG_TRIGGER_SOURCE_H_INCLUDED

#include "parser/parser-expr.h"

typedef struct _GrokInstance GrokInstance;

LogParser *grok_parser_new(void);

void grok_instance_set_pattern (GrokInstance *s, gchar *pattern);
void grok_parser_add_named_subpattern(LogParser *self, const char *name, const char *pattern);
void grok_parser_set_pattern_directory(LogParser *s, gchar *pattern_directory);
void grok_parser_set_key_prefix(LogParser *s, gchar *key_prefix);
void grok_parser_add_pattern_instance(LogParser *s, GrokInstance *instance);

#endif
