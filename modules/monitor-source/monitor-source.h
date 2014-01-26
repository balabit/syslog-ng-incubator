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

#ifndef SNG_MONITOR_SOURCE_H_INCLUDED
#define SNG_MONITOR_SOURCE_H_INCLUDED

#include "driver.h"
#include "logsource.h"

LogDriver *monitor_sd_new (void);

void monitor_sd_set_monitor_freq (LogDriver *s, gint freq);
LogSourceOptions *monitor_sd_get_source_options (LogDriver *s);
void monitor_sd_set_monitor_script (LogDriver *s, const gchar *script);
void monitor_sd_set_monitor_func (LogDriver *s, const gchar *function_name);

#endif
