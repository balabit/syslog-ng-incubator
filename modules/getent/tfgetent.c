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
#include <stddef.h>

typedef gboolean (*lookup_method)(gchar *key, gchar *member_name, GString *result);
typedef gboolean (*format_member)(gchar *member_name, gpointer member, GString *result);

static gboolean
tf_getent_services(gchar *key, gchar *member_name, GString *result)
{
  struct servent serv, *res;
  glong d;
  gboolean is_num;
  char buf[4096];

  if ((is_num = parse_number(key, &d)) == TRUE)
    getservbyport_r((int)ntohs(d), NULL, &serv, buf, sizeof(buf), &res);
  else
    getservbyname_r(key, NULL, &serv, buf, sizeof(buf), &res);

  if (res == NULL)
    return TRUE;

  if (is_num)
    g_string_append(result, res->s_name);
  else
    g_string_append_printf(result, "%i", htons(res->s_port));

  return TRUE;
}

static gboolean
_getent_format_string(gchar *member_name, gpointer member, GString *result)
{
  char *value = *(char **)member;

  g_string_append(result, value);
  return TRUE;
}

static gboolean
_getent_format_uid_gid(gchar *member_name, gpointer member, GString *result)
{
  if (strcmp(member_name, "uid") == 0)
    {
      uid_t u = *(uid_t *)member;

      g_string_append_printf(result, "%" G_GUINT64_FORMAT, (guint64)u);
    }
  else
    {
      gid_t g = *(gid_t *)member;

      g_string_append_printf(result, "%" G_GUINT64_FORMAT, (guint64)g);
    }

  return TRUE;
}

typedef struct
{
  gchar *member_name;
  format_member format;
  size_t offset;
} formatter_map_t;

static formatter_map_t passwd_field_map[] = {
  { "name", _getent_format_string, offsetof(struct passwd, pw_name) },
  { "uid", _getent_format_uid_gid, offsetof(struct passwd, pw_uid) },
  { "gid", _getent_format_uid_gid, offsetof(struct passwd, pw_gid) },
  { "gecos", _getent_format_string, offsetof(struct passwd, pw_gecos) },
  { "dir", _getent_format_string, offsetof(struct passwd, pw_dir) },
  { "shell", _getent_format_string, offsetof(struct passwd, pw_shell) },
  { NULL, NULL, 0 }
};

static int
_find_formatter(formatter_map_t *map, gchar *member_name)
{
  gint i = 0;

  while (map[i].member_name != NULL)
    {
      if (strcmp(map[i].member_name, member_name) == 0)
        return i;
      i++;
    }

  return -1;
}

static gboolean
tf_getent_passwd(gchar *key, gchar *member_name, GString *result)
{
  struct passwd pwd;
  struct passwd *res;
  char *buf;
  long bufsize;
  int s;
  glong d;
  gboolean is_num, r;

  bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (bufsize == -1)
    bufsize = 16384;

  buf = g_malloc(bufsize);

  if ((is_num = parse_number(key, &d)) == TRUE)
    s = getpwuid_r((uid_t)d, &pwd, buf, bufsize, &res);
  else
    s = getpwnam_r(key, &pwd, buf, bufsize, &res);

  if (res == NULL && s != 0)
    {
      msg_error("$(getent passwd) failed",
                evt_tag_str("key", key),
                evt_tag_errno("errno", errno),
                NULL);
      g_free(buf);
      return FALSE;
    }

  if (member_name == NULL)
    {
      if (is_num)
        member_name = "name";
      else
        member_name = "uid";
    }

  if (res == NULL)
    {
      g_free(buf);
      return FALSE;
    }

  s = _find_formatter(passwd_field_map, member_name);

  if (s == -1)
    {
      msg_error("$(getent passwd): unknown member",
                evt_tag_str("key", key),
                evt_tag_str("member", member_name),
                NULL);
      g_free(buf);
      return FALSE;
    }

  r = passwd_field_map[s].format(member_name,
                                 ((uint8_t *)res) + passwd_field_map[s].offset,
                                 result);
  g_free(buf);
  return r;
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

  if (argc != 2 && argc != 3)
    {
      msg_error("$(getent) takes either two or three arguments",
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

  return lookup(argv[1]->str, (argc == 2) ? NULL : argv[2]->str, result);
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
