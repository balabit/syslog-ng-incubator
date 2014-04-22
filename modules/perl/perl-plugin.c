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
 *
 */

#include "perl-parser.h"
#include "perl-dest.h"

#include "plugin.h"
#include "plugin-types.h"

#include <EXTERN.h>
#include <perl.h>

extern CfgParser perl_parser;

static Plugin perl_plugin =
{
  .type = LL_CONTEXT_DESTINATION,
  .name = "perl",
  .parser = &perl_parser,
};

gboolean
perl_module_init(GlobalConfig *cfg, CfgArgs *args G_GNUC_UNUSED)
{
  int argc = 1;
  char *argv[] = {"syslog-ng"};
  char *env[] = {NULL};

  PERL_SYS_INIT3(&argc, (char ***)&argv, (char ***)&env);

  plugin_register(cfg, &perl_plugin, 1);
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "perl",
  .version = VERSION,
  .description = "The perl module provides Perl scripted destination support for syslog-ng.",
  .core_revision = VERSION_CURRENT_VER_ONLY,
  .plugins = &perl_plugin,
  .plugins_len = 1,
};
