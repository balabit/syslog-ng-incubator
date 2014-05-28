/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Viktor Tusa <tusavik@gmail.com>
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

static void
tf_string_padding(LogMessage *msg, gint argc, GString *argv[], GString *result)
{
  GString *text = argv[0];
  GString *padding;
  gint64 width, i;
  
  if (argc <= 1)
    {
      msg_debug("Not enough arguments for padding template function!", NULL);
      return;
    }

  if (!parse_number_with_suffix(argv[1]->str, &width))
    {
      msg_debug("Padding template function requires a number as second argument!", NULL);
      return;
    }

  if (argc <= 2)
    padding = g_string_new(" "); 
  else
    padding = argv[2];

  if (text->len < width)
    {
      for (i = 0; i < width - text->len; i++)
        {
          g_string_append_c(result, *(padding->str + (i % padding->len)));
        }  
    }

  g_string_append_len(result, text->str, text->len);

  if (argc <= 2)
    {
      g_string_free(padding, TRUE);
    }
}

TEMPLATE_FUNCTION_SIMPLE(tf_string_padding);
