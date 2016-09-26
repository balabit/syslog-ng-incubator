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

#include "trigger-source.h"

#include "driver.h"
#include "logsource.h"
#include "mainloop.h"

#include <iv.h>

typedef struct
{
  gint trigger_freq;
  gchar *message;
} TriggerOptions;

typedef struct _TriggerSourceDriver
{
  LogSrcDriver super;
  LogSource *source;
  LogSourceOptions source_options;

  TriggerOptions options;
} TriggerSourceDriver;

typedef struct
{
  LogSource super;
  struct iv_timer trigger_timer;
  gboolean watches_running;

  TriggerOptions *options;
} TriggerSource;

#ifndef SCS_TRIGGER
#define SCS_TRIGGER 0
#endif

/*
 * TriggerSource
 */

static void trigger_source_update_watches (TriggerSource *self);

static void
trigger_source_triggered (gpointer s)
{
  TriggerSource *self = (TriggerSource *) s;
  LogMessage *msg;
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;

  main_loop_assert_main_thread ();

  if (!log_source_free_to_send (&self->super))
    goto end;

  msg = log_msg_new_internal (LOG_INFO, self->options->message);
  path_options.ack_needed = FALSE;

  log_pipe_queue (&self->super.super, msg, &path_options);

 end:
  trigger_source_update_watches (self);
}

static void
trigger_source_init_watches (TriggerSource *self)
{
  IV_TIMER_INIT (&self->trigger_timer);
  self->trigger_timer.cookie = self;
  self->trigger_timer.handler = trigger_source_triggered;
}

static void
trigger_source_start_watches (TriggerSource *self)
{
  if (self->watches_running)
    return;

  if (self->trigger_timer.expires.tv_sec >= 0)
    iv_timer_register (&self->trigger_timer);
  self->watches_running = TRUE;
}

static void
trigger_source_stop_watches (TriggerSource *self)
{
  if (!self->watches_running)
    return;

  if (iv_timer_registered (&self->trigger_timer))
    iv_timer_unregister (&self->trigger_timer);

  self->watches_running = FALSE;
}

static void
trigger_source_update_watches (TriggerSource *self)
{
  if (!log_source_free_to_send (&self->super))
    {
      trigger_source_stop_watches (self);
      return;
    }

  iv_validate_now ();
  trigger_source_stop_watches (self);
  self->trigger_timer.expires = iv_now;
  self->trigger_timer.expires.tv_sec += self->options->trigger_freq;
  trigger_source_start_watches (self);
}

static gboolean
trigger_source_init (LogPipe *s)
{
  TriggerSource *self = (TriggerSource *)s;

  if (!log_source_init (s))
    return FALSE;

  iv_validate_now ();
  self->trigger_timer.expires = iv_now;
  self->trigger_timer.expires.tv_sec += self->options->trigger_freq;

  trigger_source_start_watches (self);

  return TRUE;
}

static gboolean
trigger_source_deinit (LogPipe *s)
{
  TriggerSource *self = (TriggerSource *)s;

  trigger_source_stop_watches (self);

  return log_source_deinit (s);
}


static LogSource *
trigger_source_new (TriggerSourceDriver *owner, LogSourceOptions *options, GlobalConfig *cfg)
{
  TriggerSource *self = g_new0 (TriggerSource, 1);

  log_source_init_instance (&self->super, cfg);
  log_source_set_options (&self->super, options, 0, SCS_TRIGGER,
                          owner->super.super.id, NULL, FALSE, FALSE,
                          owner->super.super.super.expr_node);

  self->options = &owner->options;

  trigger_source_init_watches (self);

  self->super.super.init = trigger_source_init;
  self->super.super.deinit = trigger_source_deinit;

  return &self->super;
}

/*
 * TriggerSourceDriver
 */

static gboolean
trigger_sd_init (LogPipe *s)
{
  TriggerSourceDriver *self = (TriggerSourceDriver *)s;
  GlobalConfig *cfg = log_pipe_get_config (s);

  if (!log_src_driver_init_method (s))
    return FALSE;

  if (self->options.trigger_freq <= 0)
    self->options.trigger_freq = 10;

  if (!self->options.message)
    self->options.message = g_strdup ("Trigger source is trigger happy.");

  log_source_options_init (&self->source_options, cfg, self->super.super.group);
  self->source = trigger_source_new (self, &self->source_options, cfg);

  log_pipe_append (&self->source->super, s);
  log_pipe_init (&self->source->super);

  return TRUE;
}

static gboolean
trigger_sd_deinit (LogPipe *s)
{
  TriggerSourceDriver *self = (TriggerSourceDriver *)s;

  if (self->source)
    {
      log_pipe_deinit (&self->source->super);
      log_pipe_unref (&self->source->super);
      self->source = NULL;
    }

  g_free (self->options.message);

  return log_src_driver_deinit_method (s);
}

LogSourceOptions *
trigger_sd_get_source_options (LogDriver *s)
{
  TriggerSourceDriver *self = (TriggerSourceDriver *)s;

  return &self->source_options;
}

void
trigger_sd_set_trigger_freq (LogDriver *s, gint freq)
{
  TriggerSourceDriver *self = (TriggerSourceDriver *)s;

  self->options.trigger_freq = freq;
}

void
trigger_sd_set_trigger_message (LogDriver *s, const gchar *message)
{
  TriggerSourceDriver *self = (TriggerSourceDriver *)s;

  g_free (self->options.message);
  self->options.message = g_strdup (message);
}

LogDriver *
trigger_sd_new (GlobalConfig *cfg)
{
  TriggerSourceDriver *self = g_new0 (TriggerSourceDriver, 1);

  log_src_driver_init_instance ((LogSrcDriver *)&self->super, cfg);

  self->super.super.super.init = trigger_sd_init;
  self->super.super.super.deinit = trigger_sd_deinit;

  log_source_options_defaults (&self->source_options);

  return (LogDriver *)self;
}
