/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Gergely Nagy <algernon@balabit.hu>
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
 *
 */

#include "monitor-source.h"
#include "monitor-source-parser.h"

#include "plugin.h"
#include "plugin-types.h"

extern CfgParser monitor_parser;

static Plugin monitor_plugin =
{
  .type = LL_CONTEXT_SOURCE,
  .name = "monitor",
  .parser = &monitor_parser,
};

gboolean
monitor_module_init(PluginContext *context, CfgArgs *args G_GNUC_UNUSED)
{
  plugin_register(context, &monitor_plugin, 1);
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "monitor",
  .version = SYSLOG_NG_VERSION,
  .description = "The monitor module provides stuff.",
  .core_revision = VERSION_CURRENT_VER_ONLY,
  .plugins = &monitor_plugin,
  .plugins_len = 1,
};
