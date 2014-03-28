/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Gergely Nagy <algernon@balabit.hu>
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

#include <syslog-ng.h>
#include <logmsg.h>
#include <plugin.h>
#include <plugin-types.h>
#include <cfg.h>
#include <parse-number.h>

#include <pwd.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <features.h>

typedef gboolean (*lookup_method)(GString *key, GString *result);

static gboolean
tf_getent_services(GString *key, GString *result)
{
  struct servent serv, *res;
  glong d;
  gboolean is_num;
  char buf[4096];

  if ((is_num = parse_number(key->str, &d)) == TRUE)
    getservbyport_r((int)ntohs(d), NULL, &serv, buf, sizeof(buf), &res);
  else
    getservbyname_r(key->str, NULL, &serv, buf, sizeof(buf), &res);

  if (res == NULL)
    return TRUE;

  if (is_num)
    g_string_append(result, res->s_name);
  else
    g_string_append_printf(result, "%i", htons(res->s_port));

  return TRUE;
}

static gboolean
tf_getent_passwd(GString *key, GString *result)
{
  struct passwd pwd;
  struct passwd *res;
  char *buf;
  long bufsize;
  int s;
  glong d;
  gboolean is_num;

  bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (bufsize == -1)
    bufsize = 16384;

  buf = g_malloc(bufsize);

  if ((is_num = parse_number(key->str, &d)) == TRUE)
    s = getpwuid_r((uid_t)d, &pwd, buf, bufsize, &res);
  else
    s = getpwnam_r(key->str, &pwd, buf, bufsize, &res);

  if (res == NULL && s != 0)
    {
      msg_error("$(getent passwd) failed",
                evt_tag_str("key", key->str),
                evt_tag_errno("errno", errno),
                NULL);
      g_free(buf);
      return FALSE;
    }

  if (res != NULL)
    {
      if (is_num)
        g_string_append(result, res->pw_name);
      else
        g_string_append_printf(result, "%lu", (unsigned long)res->pw_uid);
    }

  g_free(buf);

  return TRUE;
}

static struct
{
  gchar *entity;
  lookup_method lookup;
} tf_getent_lookup_map[] =  {
  { "passwd", tf_getent_passwd },
  { "services", tf_getent_services },
  { NULL, NULL }
};

static lookup_method
tf_getent_find_lookup_method(gchar *entity)
{
  gint i = 0;

  while (tf_getent_lookup_map[i].entity != NULL)
    {
      if (strcmp(tf_getent_lookup_map[i].entity, entity) == 0)
        return tf_getent_lookup_map[i].lookup;
      i++;
    }
  return NULL;
}

static gboolean
tf_getent(LogMessage *msg, gint argc, GString *argv[], GString *result)
{
  lookup_method lookup;

  if (argc != 2)
    {
      msg_error("$(getent) takes exactly two arguments",
                evt_tag_int("argc", argc),
                NULL);
      return FALSE;
    }

  lookup = tf_getent_find_lookup_method(argv[0]->str);
  if (!lookup)
    {
      msg_error("Unsupported $(getent) NSS service",
                evt_tag_str("service", argv[0]->str),
                NULL);
      return FALSE;
    }

  return lookup(argv[1], result);
}
TEMPLATE_FUNCTION_SIMPLE(tf_getent);

static Plugin getent_plugins[] =
{
  TEMPLATE_FUNCTION_PLUGIN(tf_getent, "getent"),
};

gboolean
getent_plugin_module_init(GlobalConfig *cfg, CfgArgs *args)
{
  plugin_register(cfg, getent_plugins, G_N_ELEMENTS(getent_plugins));
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "getent-plugin",
  .version = VERSION,
  .description = "The getent module provides getent template functions for syslog-ng.",
  .core_revision = VERSION_CURRENT_VER_ONLY,
  .plugins = getent_plugins,
  .plugins_len = G_N_ELEMENTS(getent_plugins),
};
