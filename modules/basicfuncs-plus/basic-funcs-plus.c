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

#include <math.h>
#include <errno.h>
#include <stdlib.h>

#include "plugin.h"
#include "cfg.h"
#include "parse-number.h"
#include "plugin-types.h"

static gboolean
tf_num_parse(gint argc, GString *argv[],
             const gchar *func_name, glong *n, glong *m)
{
  if (argc != 2)
    {
      msg_debug("Template function requires two arguments.",
                evt_tag_str("function", func_name), NULL);
      return FALSE;
    }

  if (!parse_number_with_suffix(argv[0]->str, n))
    {
      msg_debug("Parsing failed, template function's first argument is not a number",
                evt_tag_str("function", func_name),
                evt_tag_str("arg1", argv[0]->str), NULL);
      return FALSE;
    }

  if (!parse_number_with_suffix(argv[1]->str, m))
    {
      msg_debug("Parsing failed, template function's first argument is not a number",
                evt_tag_str("function", func_name),
                evt_tag_str("arg1", argv[1]->str), NULL);
      return FALSE;
    }

  return TRUE;
}

static void
tf_num_divx(LogMessage *msg, gint argc, GString *argv[], GString *result)
{
  glong n, m;

  if (!tf_num_parse(argc, argv, "//", &n, &m) || !m)
    {
      g_string_append_len(result, "NaN", 3);
      return;
    }

  g_string_append_printf (result, "%f", (double) ((double)n / (double)m));
}

TEMPLATE_FUNCTION_SIMPLE(tf_num_divx);

/*
 * Plugin glue
 */

static Plugin basicfuncs_plus_plugins[] =
{
  TEMPLATE_FUNCTION_PLUGIN(tf_num_divx, "//"),
};

gboolean
basicfuncs_plus_module_init(GlobalConfig *cfg, CfgArgs *args)
{
  plugin_register(cfg, basicfuncs_plus_plugins,
                  G_N_ELEMENTS(basicfuncs_plus_plugins));
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "basicfuncs-plus",
  .version = VERSION,
  .description = "The basicfuncs-plus module provides some additional template functions for syslog-ng.",
  .core_revision = VERSION_CURRENT_VER_ONLY,
  .plugins = basicfuncs_plus_plugins,
  .plugins_len = G_N_ELEMENTS(basicfuncs_plus_plugins),
};
