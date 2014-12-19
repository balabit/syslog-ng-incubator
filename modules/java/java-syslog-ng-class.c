/*
 * Copyright (c) 2010-2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2010-2014 Viktor Juhasz <viktor.juhasz@balabit.com>
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

#include "java-syslog-ng-class.h"
#include "messages.h"

#define SYSLOG_NG_CLASS         "org.syslog_ng.SyslogNg"

#define CALL_JAVA_FUNCTION(env, function, ...) (*(env))->function(env, __VA_ARGS__)

SyslogNgClass *
syslog_ng_class_new(JNIEnv *java_env, ClassLoader *loader, gpointer ptr)
{
  SyslogNgClass *self = g_new0(SyslogNgClass, 1);

  self->syslogng_class = class_loader_load_class(loader, java_env, SYSLOG_NG_CLASS);
  if (!self->syslogng_class)
    {
      (*java_env)->ExceptionDescribe(java_env);
      msg_error("Can't find SyslogNg class", NULL);
      goto error;
    }

  self->syslogng_constructor_id = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->syslogng_class, "<init>", "(J)V");
  if (!self->syslogng_constructor_id)
    {
      msg_error("Can't find method in class",
                evt_tag_str("class_name", SYSLOG_NG_CLASS),
                evt_tag_str("method", "SyslogNg(long)"),
                NULL);
      goto error;
    }
  self->syslogng_object = CALL_JAVA_FUNCTION(java_env,
                                             NewObject,
                                             self->syslogng_class,
                                             self->syslogng_constructor_id,
                                             ptr);
  if (!self->syslogng_object)
    {
      msg_error("Failed to create SyslogNg object", NULL);
      goto error;
    }
  return self;
error:
  syslog_ng_class_free(self, java_env);
  return NULL;
}

void
syslog_ng_class_free(SyslogNgClass *self, JNIEnv *java_env)
{
  if (!self)
    {
      return;
    }
  if (self->syslogng_object)
    {
      CALL_JAVA_FUNCTION(java_env, DeleteLocalRef, self->syslogng_object);
    }
  if (self->syslogng_class)
    {
      CALL_JAVA_FUNCTION(java_env, DeleteLocalRef, self->syslogng_class);
    }
  g_free(self);
}
