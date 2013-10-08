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

#ifndef MYSQL_MOD_H_INCLUDED
#define MYSQL_MOD_H_INCLUDED

#include "driver.h"

enum
{
  MYSQL_COLUMN_DEFAULT = 1,
};

void mysql_dd_set_host(LogDriver *s, const gchar *host);
void mysql_dd_set_port(LogDriver *s, gint port);
void mysql_dd_set_user(LogDriver *s, const gchar *user);
void mysql_dd_set_password(LogDriver *s, const gchar *password);
void mysql_dd_set_database(LogDriver *s, const gchar *database);
void mysql_dd_set_table(LogDriver *s, const gchar *table);
void mysql_dd_set_columns(LogDriver *s, GList *columns);
void mysql_dd_set_values(LogDriver *s, GList *values);
void mysql_dd_set_null_value(LogDriver *s, const gchar *null);
void mysql_dd_set_indexes(LogDriver *s, GList *indexes);
void mysql_dd_set_retries(LogDriver *s, gint num_retries);
void mysql_dd_set_flush_lines(LogDriver *s, gint flush_lines);
void mysql_dd_set_flush_timeout(LogDriver *s, gint flush_timeout);
void mysql_dd_set_session_statements(LogDriver *s, GList *session_statements);
void mysql_dd_set_bulk_insert(LogDriver *s, gboolean bulk_insert);
void mysql_dd_set_retries(LogDriver *s, gint num_retries);

LogDriver *mysql_dd_new();

#endif
