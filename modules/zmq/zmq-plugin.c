/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Laszlo Meszaros <lacienator@gmail.com>
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

#include <zmq.h>

#include "zmq-parser.h"
#include "plugin.h"
#include "messages.h"
#include "misc.h"
#include "stats/stats.h"
#include "logqueue.h"
#include "driver.h"
#include "plugin-types.h"

extern CfgParser zmq_dd_parser;

static Plugin zmq_plugins[] =
{
  {
    .type = LL_CONTEXT_SOURCE,
    .name = "zmq",
    .parser = &zmq_parser,
  },
  {
    .type = LL_CONTEXT_DESTINATION,
    .name = "zmq",
    .parser = &zmq_parser,
  },
};

gboolean
zmq_module_init(GlobalConfig *cfg, CfgArgs *args)
{
  plugin_register(cfg, zmq_plugins, G_N_ELEMENTS(zmq_plugins));
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "zmq",
  .version = SYSLOG_NG_VERSION,
  .description = "The zmq module provides ZeroMQ destination support for syslog-ng.",
  .core_revision = "Dummy Revision",
  .plugins = zmq_plugins,
  .plugins_len = G_N_ELEMENTS(zmq_plugins),
};
