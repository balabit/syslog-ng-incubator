/*
 * Copyright (c) 2011 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2011 Gergely Nagy <algernon@balabit.hu>
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

#include <glib.h>
#include <mongo.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static const gdouble golden_ratio = 0.618033988749895;

typedef struct
{
  gchar *host;
  gint port;
  gchar *db;
  gchar *coll;
  gchar *output;
  gchar *sort_by;

  struct
  {
    gchar *timestamp;
    gchar *username;
    gchar *filename;
    gchar *color_by;
  } template;

  gboolean verbose;

  /* Computed settings */
  gchar *ns;
} config_t;

static void
hsv_to_rgb (gdouble *r, gdouble *g, gdouble *b,
            gdouble h, gdouble s, gdouble v)
{
  gint i;
  gdouble f, p, q, t;

  if (s == 0)
    {
      *r = *g = *b = v;
      return;
    }

  h /= 60;
  i = floor (h);

  f = h - i;
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));

  switch (i)
    {
    case 0:
      *r = v;
      *g = t;
      *b = p;
      break;
    case 1:
      *r = q;
      *g = v;
      *b = p;
      break;
    case 2:
      *r = p;
      *g = v;
      *b = t;
      break;
    case 3:
      *r = p;
      *g = q;
      *b = v;
      break;
    case 4:
      *r = t;
      *g = p;
      *b = v;
      break;
    default:
      *r = v;
      *g = p;
      *b = q;
      break;
    }
}

static gchar *
lmg_color_next (gdouble *h)
{
  gdouble r, g, b;

  *h += golden_ratio;
  *h = fmod (*h, 1.0);

  hsv_to_rgb (&r, &g, &b,
              (*h) * 360, 0.95, 0.95);

  return g_strdup_printf ("%02X%02X%02X",
                          (gint)(r * 255), (gint)(g * 255), (gint)(b * 255));
}

static gboolean
lmg_regex_eval_cb (const GMatchInfo *info,
                   GString *res,
                   gpointer data)
{
  bson *msg = (bson *)data;
  bson_cursor *c;
  gchar *match;
  const gchar *r;

  match = g_match_info_fetch (info, 1);
  c = bson_find (msg, match);
  if (!c)
    {
      g_free (match);
      return FALSE;
    }
  if (!bson_cursor_get_string (c, &r))
    {
      g_free (match);
      bson_cursor_free (c);
      return FALSE;
    }
  g_string_append (res, r);
  g_free (match);
  bson_cursor_free (c);
  return FALSE;
}

int
main (int argc, char *argv[])
{
  mongo_sync_connection *conn;
  mongo_sync_cursor *cursor;
  mongo_packet *p;
  bson *query, *empty;

  GHashTable *host_colors;
  gdouble h;

  gint64 cnt, pos = 0;
  GRand *rnd;

  GError *error = NULL;
  GOptionContext *context;
  config_t config;
  GOptionEntry entries[] =
    {
      { "host", 'h', 0, G_OPTION_ARG_STRING, &config.host,
        "Host to connect to", "HOST" },
      { "port", 'p', 0, G_OPTION_ARG_INT, &config.port, "Port to connect to",
        "PORT" },
      { "db", 'd', 0, G_OPTION_ARG_STRING, &config.db, "Database to use",
        "DB" },
      { "collection", 'c', 0, G_OPTION_ARG_STRING, &config.coll,
        "Collection tthat stores the log messages", "COLL" },
      { "sort-by", 's', 0, G_OPTION_ARG_STRING, &config.sort_by,
        "Field to sort on", "FIELD" },

      { "template-timestamp", 'T', 0, G_OPTION_ARG_STRING,
        &config.template.timestamp, "Timestamp template", "TEMPLATE" },
      { "template-username", 'U', 0, G_OPTION_ARG_STRING,
        &config.template.username, "Username template", "TEMPLATE" },
      { "template-filename", 'F', 0, G_OPTION_ARG_STRING,
        &config.template.filename, "Filename template", "TEMPLATE" },
      { "template-color-by", 'C', 0, G_OPTION_ARG_STRING,
        &config.template.color_by, "Color key template", "TEMPLATE" },

      { "output", 'o', 0, G_OPTION_ARG_STRING, &config.output,
        "Output file", "FILENAME" },
      { "verbose", 'v', 0, G_OPTION_ARG_NONE, &config.verbose,
        "Be verbose", NULL },
      { NULL, 0, 0, 0, NULL, NULL, NULL }
    };

  GRegex *reg;
  FILE *out;

  memset (&config, 0, sizeof (config));

  context = g_option_context_new ("- MongoDB log to Gource custom format exporter");
  g_option_context_add_main_entries (context, entries, "logmongource");
  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_print ("option parsing failed: %s\n", error->message);
      exit (1);
    }

  if (!config.host)
    config.host = g_strdup ("localhost");
  if (!config.port)
    config.port = 27017;
  if (!config.db)
    config.db = g_strdup ("syslog");
  if (!config.coll)
    config.coll = g_strdup ("messages");
  if (!config.sort_by)
    config.sort_by = g_strdup ("DATE");

  if (!config.template.timestamp)
    config.template.timestamp = g_strdup ("${DATE}");
  if (!config.template.username)
    config.template.username = g_strdup ("${HOST}@${SOURCE}");
  if (!config.template.filename)
    config.template.filename = g_strdup ("${HOST}/${PROGRAM}/msgs");

  if (!config.output)
    {
      gchar *help = g_option_context_get_help (context, TRUE, NULL);

      printf ("%s", help);
      exit (1);
    }

  config.ns = g_strdup_printf ("%s.%s", config.db, config.coll);

  rnd = g_rand_new ();
  h = g_rand_double (rnd);
  g_rand_free (rnd);

  conn = mongo_sync_connect (config.host, config.port, TRUE);
  if (!conn)
    {
      perror ("logmongource/mongo_sync_connect()");
      exit (1);
    }

  empty = bson_new ();
  bson_finish (empty);

  query = bson_build_full (BSON_TYPE_DOCUMENT, "$query", FALSE,
                           empty,
                           BSON_TYPE_DOCUMENT, "$orderby", TRUE,
                           bson_build (BSON_TYPE_INT32, config.sort_by, 1,
                                       BSON_TYPE_NONE),
                           BSON_TYPE_NONE);
  bson_finish (query);

  cnt = (gint64)mongo_sync_cmd_count (conn, config.db, config.coll, empty);
  bson_free (empty);
  if (cnt < 0)
    {
      perror ("logmongource/mongo_sync_cmd_count()");
      exit (1);
    }

  p = mongo_sync_cmd_query (conn, config.ns, 0,
                            0, 0, query, NULL);
  bson_free (query);
  if (!p)
    {
      perror ("logmongource/mongo_sync_cmd_query()");
      exit (1);
    }

  cursor = mongo_sync_cursor_new (conn, config.ns, p);
  if (!cursor)
    {
      perror ("logmongource/mongo_sync_cursor_new()");
      exit (1);
    }

  host_colors = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);

  reg = g_regex_new ("\\$\\{([^\\}]+)\\}", G_REGEX_OPTIMIZE, 0, &error);
  if (!reg)
    {
      g_print ("logmongource/g_regex_new(): %s\n", error->message);
      exit (1);
    }

  if (strcmp (config.output, "-") == 0)
    out = stdout;
  else
    {
      out = fopen (config.output, "w");
      if (!out)
        {
          perror ("logmongource/fopen()");
          exit (1);
        }
    }

  while (mongo_sync_cursor_next (cursor) && pos < cnt)
    {
      bson *data;
      gchar *ts, *user, *file, *color_by, *hc;

      data = mongo_sync_cursor_get_data (cursor);

      ts = g_regex_replace_eval (reg, config.template.timestamp,
                                 strlen (config.template.timestamp), 0, 0,
                                 lmg_regex_eval_cb, data, &error);
      user = g_regex_replace_eval (reg, config.template.username,
                                   strlen (config.template.username), 0, 0,
                                   lmg_regex_eval_cb, data, &error);
      file = g_regex_replace_eval (reg, config.template.filename,
                                   strlen (config.template.filename), 0, 0,
                                   lmg_regex_eval_cb, data, &error);

      if (config.template.color_by)
        {
          color_by = g_regex_replace_eval (reg, config.template.color_by,
                                           strlen (config.template.color_by), 0, 0,
                                           lmg_regex_eval_cb, data, &error);

          if (!color_by)
            {
              fprintf (out, "%s|%s|M|%s\n", ts, user, file);
              break;
            }

          hc = g_hash_table_lookup (host_colors, color_by);
          if (!hc)
            {
              hc = lmg_color_next (&h);
              g_hash_table_insert (host_colors, g_strdup (color_by), hc);
            }
          fprintf (out, "%s|%s|M|%s|%s\n", ts, user, file, hc);
          g_free (color_by);
        }
      else
        fprintf (out, "%s|%s|M|%s\n", ts, user, file);

      bson_free (data);
      g_free (ts);
      g_free (user);
      g_free (file);

      if (pos % 10 == 0 && config.verbose)
        {
          fprintf (stderr, "\r%03.02f%%", ((pos * 1.0 / cnt) * 100));
          fflush (stderr);
        }
      pos++;
    }
  if (config.verbose)
    fprintf (stderr, "\r%02.02f%%\n", ((pos * 1.0 / cnt) * 100));

  if (out != stdout)
    fclose (out);

  mongo_sync_cursor_free (cursor);
  mongo_sync_disconnect (conn);

  g_hash_table_destroy (host_colors);
  g_regex_unref (reg);
  g_free (config.host);
  g_free (config.db);
  g_free (config.coll);
  g_free (config.output);
  g_free (config.sort_by);
  g_free (config.template.timestamp);
  g_free (config.template.username);
  g_free (config.template.filename);
  g_free (config.ns);

  g_option_context_free (context);

  return 0;
}
