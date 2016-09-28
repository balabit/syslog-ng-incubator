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

#include "perl-dest.h"
#include "logthrdestdrv.h"
#include "stats/stats.h"
#include "seqnum.h"

#include <EXTERN.h>
#include <perl.h>

#ifndef SCS_PERL
#define SCS_PERL 0
#endif

typedef struct
{
  LogThrDestDriver super;

  gchar *filename;
  gchar *init_func_name;
  gchar *queue_func_name;
  gchar *deinit_func_name;
  LogTemplateOptions template_options;
  ValuePairs *vp;

  gint32 seq_num;

  PerlInterpreter *perl;
} PerlDestDriver;

/** Setters & config glue **/

void
perl_dd_set_init_func(LogDriver *d, gchar *init_func_name)
{
  PerlDestDriver *self = (PerlDestDriver *)d;

  g_free(self->init_func_name);
  self->init_func_name = g_strdup(init_func_name);
}

void
perl_dd_set_queue_func(LogDriver *d, gchar *queue_func_name)
{
  PerlDestDriver *self = (PerlDestDriver *)d;

  g_free(self->queue_func_name);
  self->queue_func_name = g_strdup(queue_func_name);
}

void
perl_dd_set_deinit_func(LogDriver *d, gchar *deinit_func_name)
{
  PerlDestDriver *self = (PerlDestDriver *)d;

  g_free(self->deinit_func_name);
  self->deinit_func_name = g_strdup(deinit_func_name);
}

void
perl_dd_set_filename(LogDriver *d, gchar *filename)
{
  PerlDestDriver *self = (PerlDestDriver *)d;

  g_free(self->filename);
  self->filename = g_strdup(filename);
}

void
perl_dd_set_value_pairs(LogDriver *d, ValuePairs *vp)
{
  PerlDestDriver *self = (PerlDestDriver *)d;

  if (self->vp)
    value_pairs_unref(self->vp);
  self->vp = vp;
}

LogTemplateOptions *
perl_dd_get_template_options(LogDriver *d)
{
  PerlDestDriver *self = (PerlDestDriver *)d;

  return &self->template_options;
}

/** Helpers for stats & persist_name formatting **/

static gchar *
perl_dd_format_stats_instance(LogThrDestDriver *d)
{
  PerlDestDriver *self = (PerlDestDriver *)d;
  static gchar persist_name[1024];

  if (d->super.super.super.persist_name)
    g_snprintf(persist_name, sizeof(persist_name), "perl,%s", d->super.super.super.persist_name);
  else
    g_snprintf(persist_name, sizeof(persist_name),
               "perl,%s,%s,%s,%s",
               self->filename,
               self->init_func_name,
               self->queue_func_name,
               self->deinit_func_name);
  return persist_name;
}

static const gchar *
perl_dd_format_persist_name(const LogPipe *d)
{
  const PerlDestDriver *self = (const PerlDestDriver *)d;
  static gchar persist_name[1024];

  if (d->persist_name)
    g_snprintf(persist_name, sizeof(persist_name), "perl.%s", d->persist_name);
  else
    g_snprintf(persist_name, sizeof(persist_name),
               "perl(%s,%s,%s,%s)",
               self->filename,
               self->init_func_name,
               self->queue_func_name,
               self->deinit_func_name);
  return persist_name;
}

/** Perl calling helpers **/

static gboolean
_call_perl_function_with_no_arguments(PerlDestDriver *self, const gchar *fname)
{
  PerlInterpreter *my_perl = self->perl;
  char *args[] = { NULL };
  dSP;
  int count, r = 0;

  ENTER;
  SAVETMPS;

  count = call_argv(fname, G_SCALAR | G_EVAL | G_NOARGS, args);

  SPAGAIN;

  if (SvTRUE(ERRSV))
    {
      msg_error("Error while calling a Perl function",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                evt_tag_str("function", fname),
                evt_tag_str("error-message", SvPV_nolen(ERRSV)),
                NULL);
      (void) POPs;
      goto exit;
    }

  if (count != 1)
    {
      msg_error("Too many values returned by a Perl function",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                evt_tag_str("function", fname),
                evt_tag_int("returned-values", count),
                evt_tag_int("expected-values", 1),
                NULL);
      return FALSE;
    }

  r = POPi;

 exit:
  PUTBACK;
  FREETMPS;
  LEAVE;

  return (r != 0);
}

static void xs_init (pTHX);

EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);

EXTERN_C void
xs_init(pTHX)
{
  char *file = __FILE__;
  dXSUB_SYS;

  /* DynaLoader is a special case */
  newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
}

/** Value pairs **/

static gboolean
perl_worker_vp_add_one(const gchar *name,
                       TypeHint type, const gchar *value, gsize value_len,
                       gpointer user_data)
{
  PerlInterpreter *my_perl = (PerlInterpreter *)((gpointer *)user_data)[0];
  HV *kvmap = (HV *)((gpointer *)user_data)[1];
  PerlDestDriver *self = (PerlDestDriver *)((gpointer *)user_data)[2];
  gboolean need_drop = FALSE;
  gboolean fallback = self->template_options.on_error & ON_ERROR_FALLBACK_TO_STRING;

  switch (type)
    {
    case TYPE_HINT_INT32:
      {
        gint32 i;

        if (type_cast_to_int32(value, &i, NULL))
          hv_store(kvmap, name, strlen(name), newSViv(i), 0);
        else
          {
            need_drop = type_cast_drop_helper(self->template_options.on_error,
                                              value, "int");

            if (fallback)
              hv_store(kvmap, name, strlen(name), newSVpv(value, 0), 0);
          }
        break;
      }
    case TYPE_HINT_STRING:
      hv_store(kvmap, name, strlen(name), newSVpv(value, 0), 0);
      break;
    default:
      need_drop = type_cast_drop_helper(self->template_options.on_error,
                                        value, "<unknown>");
      break;
    }
  return need_drop;
}

/** Main code **/

static void
_perl_thread_init(LogThrDestDriver *d)
{
  PerlDestDriver *self = (PerlDestDriver *)d;
  PerlInterpreter *my_perl;
  char *argv[] = { "syslog-ng", self->filename };

  self->perl = perl_alloc();
  perl_construct(self->perl);
  my_perl = self->perl;
  PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
  perl_parse(self->perl, xs_init, 2, (char **)argv, NULL);

  if (!self->queue_func_name)
    self->queue_func_name = g_strdup("queue");

  if (self->init_func_name)
    _call_perl_function_with_no_arguments(self, self->init_func_name);

  msg_verbose("Initializing Perl destination",
              evt_tag_str("driver", self->super.super.super.id),
              evt_tag_str("script", self->filename),
              NULL);
}

static worker_insert_result_t
perl_worker_eval(LogThrDestDriver *d, LogMessage *msg)
{
  PerlDestDriver *self = (PerlDestDriver *)d;
  gboolean success, vp_ok;
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;
  PerlInterpreter *my_perl = self->perl;
  int count;
  HV *kvmap;
  gpointer args[3];
  dSP;

  ENTER;
  SAVETMPS;

  PUSHMARK(SP);

  kvmap = newHV();

  args[0] = self->perl;
  args[1] = kvmap;
  args[2] = self;
  vp_ok = value_pairs_foreach(self->vp, perl_worker_vp_add_one,
                              msg, self->seq_num, LTZ_SEND,
                              &self->template_options,
                              args);

  if (!vp_ok && (self->template_options.on_error & ON_ERROR_DROP_MESSAGE))
    goto exit;

  XPUSHs(sv_2mortal(newRV_noinc((SV *)kvmap)));

  PUTBACK;

  count = call_pv(self->queue_func_name, G_EVAL | G_SCALAR);

  SPAGAIN;

  if (SvTRUE(ERRSV))
    {
      msg_error("Error while calling a Perl function",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                evt_tag_str("function", self->queue_func_name),
                evt_tag_str("error-message", SvPV_nolen(ERRSV)),
                NULL);
      (void) POPs;
      success = FALSE;
    }

  if (count != 1)
    {
      msg_error("Too many values returned by a Perl function",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                evt_tag_str("function", self->queue_func_name),
                evt_tag_int("returned-values", count),
                evt_tag_int("expected-values", 1),
                NULL);
      success = FALSE;
    }
  else
    {
      int r = POPi;

      success = (r != 0);
    }

 exit:
  PUTBACK;
  FREETMPS;
  LEAVE;

  if (success && vp_ok)
    {
      return WORKER_INSERT_RESULT_SUCCESS;
    }
  else
    {
      return WORKER_INSERT_RESULT_DROP;
    }
}

static gboolean
perl_worker_init(LogPipe *d)
{
  PerlDestDriver *self = (PerlDestDriver *)d;
  GlobalConfig *cfg = log_pipe_get_config(d);

  if (!self->filename)
    {
      msg_error("Error initializing Perl destination: no script specified!",
                evt_tag_str("driver", self->super.super.super.id),
                NULL);
      return FALSE;
    }

  if (!log_dest_driver_init_method(d))
    return FALSE;

  log_template_options_init(&self->template_options, cfg);

  return log_threaded_dest_driver_start(d);
}

static void
_perl_thread_deinit(LogThrDestDriver *d)
{
  PerlDestDriver *self = (PerlDestDriver *)d;

  if (self->deinit_func_name &&
      !_call_perl_function_with_no_arguments(self, self->deinit_func_name))
    return;

  perl_destruct(self->perl);
  perl_free(self->perl);
}

static void
perl_dd_free(LogPipe *d)
{
  PerlDestDriver *self = (PerlDestDriver *)d;

  log_template_options_destroy(&self->template_options);

  g_free(self->filename);
  g_free(self->init_func_name);
  g_free(self->queue_func_name);
  g_free(self->deinit_func_name);

  if (self->vp)
    value_pairs_unref(self->vp);

  log_threaded_dest_driver_free(d);
}

LogDriver *
perl_dd_new(GlobalConfig *cfg)
{
  PerlDestDriver *self = g_new0(PerlDestDriver, 1);

  log_threaded_dest_driver_init_instance(&self->super, cfg);

  self->super.super.super.super.init = perl_worker_init;
  self->super.super.super.super.free_fn = perl_dd_free;
  self->super.super.super.super.generate_persist_name = perl_dd_format_persist_name;

  self->super.worker.thread_init = _perl_thread_init;
  self->super.worker.thread_deinit = _perl_thread_deinit;
  self->super.worker.disconnect = NULL;
  self->super.worker.insert = perl_worker_eval;

  self->super.format.stats_instance = perl_dd_format_stats_instance;
  self->super.stats_source = SCS_PERL;

  init_sequence_number(&self->seq_num);

  log_template_options_defaults(&self->template_options);
  perl_dd_set_value_pairs(&self->super.super.super, value_pairs_new_default(cfg));

  return (LogDriver *)self;
}
