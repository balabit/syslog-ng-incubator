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

#include "python-dest.h"
#include "logthrdestdrv.h"
#include "stats.h"
#include "misc.h"

#include <Python.h>

#ifndef SCS_PYTHON
#define SCS_PYTHON 0
#endif

typedef struct
{
  LogThrDestDriver super;

  gchar *filename;
  gchar *init_func_name;
  gchar *queue_func_name;
  gchar *deinit_func_name;
  GList *imports;

  LogTemplateOptions template_options;
  ValuePairs *vp;

  gint32 seq_num;

  struct
  {
    PyObject *module;
    PyObject *init;
    PyObject *queue;
    PyObject *deinit;
  } py;
} PythonDestDriver;

/** Setters & config glue **/

void
python_dd_set_init_func(LogDriver *d, gchar *init_func_name)
{
  PythonDestDriver *self = (PythonDestDriver *)d;

  g_free(self->init_func_name);
  self->init_func_name = g_strdup(init_func_name);
}

void
python_dd_set_queue_func(LogDriver *d, gchar *queue_func_name)
{
  PythonDestDriver *self = (PythonDestDriver *)d;

  g_free(self->queue_func_name);
  self->queue_func_name = g_strdup(queue_func_name);
}

void
python_dd_set_deinit_func(LogDriver *d, gchar *deinit_func_name)
{
  PythonDestDriver *self = (PythonDestDriver *)d;

  g_free(self->deinit_func_name);
  self->deinit_func_name = g_strdup(deinit_func_name);
}

void
python_dd_set_filename(LogDriver *d, gchar *filename)
{
  PythonDestDriver *self = (PythonDestDriver *)d;

  g_free(self->filename);
  self->filename = g_strdup(filename);
}

void
python_dd_set_value_pairs(LogDriver *d, ValuePairs *vp)
{
  PythonDestDriver *self = (PythonDestDriver *)d;

  if (self->vp)
    value_pairs_free(self->vp);
  self->vp = vp;
}

void
python_dd_set_imports(LogDriver *d, GList *imports)
{
  PythonDestDriver *self = (PythonDestDriver *)d;

  string_list_free(self->imports);
  self->imports = imports;
}

LogTemplateOptions *
python_dd_get_template_options(LogDriver *d)
{
  PythonDestDriver *self = (PythonDestDriver *)d;

  return &self->template_options;
}

/** Helpers for stats & persist_name formatting **/

static gchar *
python_dd_format_stats_instance(LogThrDestDriver *d)
{
  PythonDestDriver *self = (PythonDestDriver *)d;
  static gchar persist_name[1024];

  g_snprintf(persist_name, sizeof(persist_name),
             "python,%s,%s,%s,%s",
             self->filename,
             self->init_func_name,
             self->queue_func_name,
             self->deinit_func_name);
  return persist_name;
}

static gchar *
python_dd_format_persist_name(LogThrDestDriver *d)
{
  PythonDestDriver *self = (PythonDestDriver *)d;
  static gchar persist_name[1024];

  g_snprintf(persist_name, sizeof(persist_name),
             "python(%s,%s,%s,%s)",
             self->filename,
             self->init_func_name,
             self->queue_func_name,
             self->deinit_func_name);
  return persist_name;
}

/** Python calling helpers **/
static gboolean
_py_check_bool_ret(PythonDestDriver *self,
                   const gchar *func_name,
                   PyObject *ret)
{
  if (!ret)
    {
      msg_error("Python function returned NULL",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                evt_tag_str("function", func_name),
                NULL);
      return FALSE;
    }

  if (ret == Py_None)
    {
      Py_DECREF(ret);
      return TRUE;
    }

  if (!PyBool_Check(ret))
    {
      msg_error("Python function returned a non-bool value",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                evt_tag_str("function", func_name),
                NULL);
      Py_DECREF(ret);
      return FALSE;
    }

  if (PyInt_AsLong(ret) != 1)
    {
      msg_error("Python function returned FALSE",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                evt_tag_str("function", func_name),
                NULL);
      Py_DECREF(ret);
      return FALSE;
    }

  return TRUE;
}

static gboolean
_py_call_noarg_boolish(PythonDestDriver *self,
                       const gchar *func_name,
                       PyObject *func)
{
  PyObject *ret;
  gboolean success;

  if (!func)
    return TRUE;

  ret = PyObject_CallObject(func, NULL);
  success = _py_check_bool_ret(self, func_name, ret);
  Py_DECREF(ret);
  return success;
}

/** Value pairs **/

static gboolean
python_worker_vp_add_one(const gchar *name,
                       TypeHint type, const gchar *value,
                       gpointer user_data)
{
  PythonDestDriver *self = (PythonDestDriver *)((gpointer *)user_data)[0];
  PyObject *dict = (PyObject *)((gpointer *)user_data)[1];
  gboolean need_drop = FALSE;
  gboolean fallback = self->template_options.on_error & ON_ERROR_FALLBACK_TO_STRING;

  switch (type)
    {
    case TYPE_HINT_INT32:
    case TYPE_HINT_INT64:
      {
        gint64 i;

        if (type_cast_to_int64(value, &i, NULL))
          PyDict_SetItemString(dict, name, PyInt_FromLong(i));
        else
          {
            need_drop = type_cast_drop_helper(self->template_options.on_error,
                                              value, "int");

            if (fallback)
              PyDict_SetItemString(dict, name, PyString_FromString(value));
          }
        break;
      }
    case TYPE_HINT_STRING:
      PyDict_SetItemString(dict, name, PyString_FromString(value));
      break;
    default:
      need_drop = type_cast_drop_helper(self->template_options.on_error,
                                        value, "<unknown>");
      break;
    }
  return need_drop;
}

/** Main code **/

static gboolean
python_worker_eval(LogThrDestDriver *d)
{
  PythonDestDriver *self = (PythonDestDriver *)d;
  gboolean success, need_drop;
  LogMessage *msg;
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;
  PyObject *dict, *ret, *pargs;
  gpointer args[2];

  success = log_queue_pop_head(self->super.queue, &msg, &path_options, FALSE, FALSE);
  if (!success)
    return TRUE;

  msg_set_context(msg);

  /** setup */
  pargs = PyTuple_New(1);
  dict = PyDict_New();

  args[0] = self;
  args[1] = dict;
  need_drop = !value_pairs_foreach(self->vp, python_worker_vp_add_one,
                                   msg, self->seq_num, &self->template_options,
                                   args);
  if (need_drop && (self->template_options.on_error & ON_ERROR_DROP_MESSAGE))
    goto exit;

  PyTuple_SetItem(pargs, 0, dict);

  ret = PyObject_CallObject(self->py.queue, pargs);
  success = _py_check_bool_ret(self, self->queue_func_name, ret);

  msg_set_context(NULL);

  Py_DECREF(pargs);
  Py_DECREF(ret);

  if (!success)
    {
      msg_error("Error while calling a Python function",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                evt_tag_str("function", self->queue_func_name),
                NULL);
    }

 exit:

  if (success && !need_drop)
    {
      stats_counter_inc(self->super.stored_messages);
      step_sequence_number(&self->seq_num);
      log_msg_ack(msg, &path_options);
      log_msg_unref(msg);
    }
  else
    {
      stats_counter_inc(self->super.dropped_messages);
      step_sequence_number(&self->seq_num);
      log_msg_ack(msg, &path_options);
      log_msg_unref(msg);
    }

  return success;
}

static void
_py_do_import(gpointer data, gpointer user_data)
{
  gchar *modname = (gchar *)data;
  PythonDestDriver *self = (PythonDestDriver *)user_data;
  PyObject *module, *modobj;

  module = PyString_FromString(modname);
  if (!module)
    {
      msg_error("Error allocating Python string",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("string", modname),
                NULL);
      return;
    }

  modobj = PyImport_Import(module);
  Py_DECREF(module);
  if (!modobj)
    {
      msg_error("Error loading Python module",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("module", modname),
                NULL);
      return;
    }
  Py_DECREF(modobj);
}

static gboolean
python_worker_init(LogPipe *d)
{
  PythonDestDriver *self = (PythonDestDriver *)d;
  GlobalConfig *cfg = log_pipe_get_config(d);
  PyObject *modname;

  if (!self->filename)
    {
      msg_error("Error initializing Python destination: no script specified!",
                evt_tag_str("driver", self->super.super.super.id),
                NULL);
      return FALSE;
    }

  if (!log_dest_driver_init_method(d))
    return FALSE;

  log_template_options_init(&self->template_options, cfg);

  if (!self->queue_func_name)
    self->queue_func_name = g_strdup("queue");

  Py_Initialize();

  g_list_foreach(self->imports, _py_do_import, self);

  modname = PyString_FromString(self->filename);
  if (!modname)
    {
      msg_error("Unable to convert filename to Python string",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                NULL);
      return FALSE;
    }

  self->py.module = PyImport_Import(modname);
  Py_DECREF(modname);

  if (!self->py.module)
    {
      msg_error("Unable to load Python script",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                NULL);
      return FALSE;
    }

  self->py.queue = PyObject_GetAttrString(self->py.module,
                                          self->queue_func_name);
  if (!self->py.queue || !PyCallable_Check(self->py.queue))
    {
      msg_error("Python queue function is not callable!",
                evt_tag_str("driver", self->super.super.super.id),
                evt_tag_str("script", self->filename),
                evt_tag_str("queue-function", self->queue_func_name),
                NULL);
      Py_DECREF(self->py.module);
      return FALSE;
    }

  self->py.init = PyObject_GetAttrString(self->py.module,
                                           self->init_func_name);
  if (self->py.init && !PyCallable_Check(self->py.init))
    {
      Py_DECREF(self->py.init);
      self->py.init = NULL;
    }
  self->py.deinit = PyObject_GetAttrString(self->py.module,
                                           self->deinit_func_name);
  if (self->py.deinit && !PyCallable_Check(self->py.deinit))
    {
      Py_DECREF(self->py.deinit);
      self->py.deinit = NULL;
    }

  if (self->py.init)
    {
      if (!_py_call_noarg_boolish(self, self->init_func_name,
                                  self->py.init))
        {
          if (self->py.init)
            Py_DECREF(self->py.init);
          if (self->py.deinit)
            Py_DECREF(self->py.deinit);
          Py_DECREF(self->py.queue);
          Py_DECREF(self->py.module);
          return FALSE;
        }
    }

  msg_verbose("Initializing Python destination",
              evt_tag_str("driver", self->super.super.super.id),
              evt_tag_str("script", self->filename),
              NULL);

  return log_threaded_dest_driver_start(d);
}

static gboolean
python_worker_deinit(LogPipe *d)
{
  PythonDestDriver *self = (PythonDestDriver *)d;

  if (self->py.deinit)
    {
      if (!_py_call_noarg_boolish(self, self->deinit_func_name,
                                  self->py.deinit))
        return FALSE;
    }

  Py_Finalize();

  return log_threaded_dest_driver_deinit_method(d);
}

static void
python_dd_free(LogPipe *d)
{
  PythonDestDriver *self = (PythonDestDriver *)d;

  log_template_options_destroy(&self->template_options);

  g_free(self->filename);
  g_free(self->init_func_name);
  g_free(self->queue_func_name);
  g_free(self->deinit_func_name);

  if (self->vp)
    value_pairs_free(self->vp);

  log_threaded_dest_driver_free(d);
}

LogDriver *
python_dd_new(GlobalConfig *cfg)
{
  PythonDestDriver *self = g_new0(PythonDestDriver, 1);

  log_threaded_dest_driver_init_instance(&self->super);

  self->super.super.super.super.init = python_worker_init;
  self->super.super.super.super.deinit = python_worker_deinit;
  self->super.super.super.super.free_fn = python_dd_free;

  self->super.worker.disconnect = NULL;
  self->super.worker.insert = python_worker_eval;

  self->super.format.stats_instance = python_dd_format_stats_instance;
  self->super.format.persist_name = python_dd_format_persist_name;
  self->super.stats_source = SCS_PYTHON;

  init_sequence_number(&self->seq_num);

  log_template_options_defaults(&self->template_options);
  python_dd_set_value_pairs(&self->super.super.super, value_pairs_new_default(cfg));

  return (LogDriver *)self;
}
