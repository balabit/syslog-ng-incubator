/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Viktor Tusa <tusa@balabit.hu>
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
 */

#include "graphite-output.h"
#include <syslog-ng.h>
#include <logmsg.h>
#include <value-pairs.h>
#include <vptransform.h>

typedef struct _TFGraphiteState
{
  TFSimpleFuncState super;
  NVHandle unix_timestamp_handle;
  ValuePairs *vp;
} TFGraphiteState;

static gboolean
tf_graphite_prepare(LogTemplateFunction *self, gpointer s, LogTemplate *parent,
    gint argc, gchar *argv[],
    GError **error)
{
  TFGraphiteState *state = (TFGraphiteState *)s;
  ValuePairsTransformSet *vpts;

  state->vp = value_pairs_new_from_cmdline (parent->cfg, argc, argv, error);
  if (!state->vp)
    return FALSE;

  state->unix_timestamp_handle = log_msg_get_value_handle("R_UNIXTIME");

  /* Always replace a leading dot with an underscore. */
  vpts = value_pairs_transform_set_new(".*");
  value_pairs_transform_set_add_func(vpts, value_pairs_new_transform_replace_prefix(".", "_"));
  value_pairs_add_transforms(state->vp, vpts);

  return TRUE;
}

typedef struct _TFGraphiteForeachUserData
{
  GString* formatted_unixtime;
  GString* result;
} TFGraphiteForeachUserData;

static gboolean
tf_graphite_foreach_func(const gchar *name, TypeHint type, const gchar *value, gpointer user_data)
{
  TFGraphiteForeachUserData *data = (TFGraphiteForeachUserData*) user_data;

  g_string_append(data->result, name);
  g_string_append_c(data->result,' ');
  g_string_append(data->result, value);
  g_string_append_c(data->result,' ');
  g_string_append(data->result, data->formatted_unixtime->str);
  g_string_append_c(data->result,'\n');

  return FALSE;
}

static gboolean
tf_graphite_format(GString *result, ValuePairs *vp, LogMessage *msg, const LogTemplateOptions *template_options, NVHandle unix_timestamp_handle)
{
  TFGraphiteForeachUserData userdata;
  gboolean return_value;

  userdata.result = result;
  userdata.formatted_unixtime = g_string_new(log_msg_get_value(msg, unix_timestamp_handle, NULL));

  return_value = value_pairs_foreach(vp, tf_graphite_foreach_func, msg, 0, template_options, &userdata);

  g_string_free(userdata.formatted_unixtime, FALSE);
  return return_value;
};

static void
tf_graphite_call(LogTemplateFunction *self, gpointer s,
       const LogTemplateInvokeArgs *args, GString *result)
{
  TFGraphiteState *state = (TFGraphiteState *)s;
  gint i;
  gboolean r = TRUE;
  gsize orig_size = result->len;

  for (i = 0; i < args->num_messages; i++)
    r &= tf_graphite_format(result, state->vp, args->messages[i], args->opts, state->unix_timestamp_handle);

  if (!r && (args->opts->on_error & ON_ERROR_DROP_MESSAGE))
    g_string_set_size(result, orig_size);
}

static void
tf_graphite_free_state(gpointer s)
{
  TFGraphiteState *state = (TFGraphiteState *)s;

  if (state->vp)
    value_pairs_free(state->vp);
  tf_simple_func_free_state(&state->super);
}

TEMPLATE_FUNCTION(TFGraphiteState, tf_graphite, tf_graphite_prepare, NULL, tf_graphite_call,
      tf_graphite_free_state, NULL);

