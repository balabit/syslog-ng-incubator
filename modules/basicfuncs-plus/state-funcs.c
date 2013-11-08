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

#include "syslog-ng.h"
#include "plugin.h"
#include "cfg.h"
#include "plugin-types.h"

static struct
{
  GRWLock lock;
  GHashTable *hashtable;
} tf_state_global_state;

static void
tf_state_get (LogMessage *msg, GString *result, GString *var)
{
  gchar *val;

  g_rw_lock_reader_lock (&tf_state_global_state.lock);
  if (!tf_state_global_state.hashtable)
    {
      g_rw_lock_reader_unlock (&tf_state_global_state.lock);
      return;
    }

  val = g_hash_table_lookup (tf_state_global_state.hashtable,
                             (gpointer)var->str);

  if (val)
    g_string_append_printf (result, "%s", val);

  g_rw_lock_reader_unlock (&tf_state_global_state.lock);
}

static void
tf_state_set (LogMessage *msg, GString *result, GString *var, GString *val)
{
  g_rw_lock_writer_lock (&tf_state_global_state.lock);
  if (!tf_state_global_state.hashtable)
    tf_state_global_state.hashtable = g_hash_table_new (g_str_hash, g_str_equal);

  g_hash_table_insert (tf_state_global_state.hashtable,
                       (gpointer)g_strdup (var->str),
                       (gpointer)g_strdup (val->str));

  g_rw_lock_writer_unlock (&tf_state_global_state.lock);

  tf_state_get (msg, result, var);
}

static void
tf_state (LogMessage *msg, gint argc, GString *argv[], GString *result)
{

  if (argc == 1)
    tf_state_get (msg, result, argv[0]);
  else if (argc == 2)
    tf_state_set (msg, result, argv[0], argv[1]);
  else
    msg_debug ("The $(state) template function requires either one or two arguments.",
               evt_tag_int("argc", argc),
               NULL);
}

TEMPLATE_FUNCTION_SIMPLE(tf_state);
