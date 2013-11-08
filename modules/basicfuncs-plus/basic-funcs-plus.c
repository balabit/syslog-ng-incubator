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

#include "plugin.h"
#include "cfg.h"
#include "plugin-types.h"

#include "number-funcs.c"
#include "state-funcs.c"

/*
 * Plugin glue
 */

static Plugin basicfuncs_plus_plugins[] =
{
  TEMPLATE_FUNCTION_PLUGIN(tf_num_divx, "//"),
  TEMPLATE_FUNCTION_PLUGIN(tf_state, "state"),
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
