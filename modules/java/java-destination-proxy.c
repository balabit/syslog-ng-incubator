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


#include "java-destination-proxy.h"
#include "java-class-loader.h"
#include "java-syslog-ng-class.h"
#include "messages.h"
#include <string.h>

#define CALL_JAVA_FUNCTION(env, function, ...) (*(env))->function(env, __VA_ARGS__)

typedef struct _JavaDestinationImpl
{
  jobject dest_object;
  jmethodID mi_constructor;
  jmethodID mi_init;
  jmethodID mi_deinit;
  jmethodID mi_queue;
  jmethodID mi_flush;
} JavaDestinationImpl;

struct _JavaDestinationProxy
{
  JavaVMSingleton *java_machine;
  jclass loaded_class;
  JavaDestinationImpl dest_impl;

  SyslogNgClass *syslog_ng_class;
};


static gboolean
__load_destination_object(JavaDestinationProxy *self, const gchar *class_name, const gchar *class_path)
{
  JNIEnv *java_env = NULL;
  java_env = java_machine_get_env(self->java_machine, &java_env);
  self->loaded_class = java_machine_load_class(self->java_machine, class_name, class_path);
  if (!self->loaded_class) {
      msg_error("Can't find class",
                evt_tag_str("class_name", class_name),
                NULL);
      return FALSE;
  }

  self->dest_impl.mi_constructor = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "<init>", "()V");
  if (!self->dest_impl.mi_constructor) {
      msg_error("Can't find default constructor for class",
                evt_tag_str("class_name", class_name), NULL);
      return FALSE;
  }

  self->dest_impl.mi_init = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "init", "(Lorg/syslog_ng/SyslogNg;)Z");
  if (!self->dest_impl.mi_init) {
      msg_error("Can't find method in class",
                evt_tag_str("class_name", class_name),
                evt_tag_str("method", "boolean init(SyslogNg)"), NULL);
      return FALSE;
  }

  self->dest_impl.mi_deinit = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "deinit", "()V");
  if (!self->dest_impl.mi_deinit) {
      msg_error("Can't find method in class",
                evt_tag_str("class_name", class_name),
                evt_tag_str("method", "void deinit()"), NULL);
      return FALSE;
  }

  self->dest_impl.mi_queue = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "queue", "(Ljava/lang/String;)Z");
  if (!self->dest_impl.mi_queue) {
      msg_error("Can't find method in class",
                evt_tag_str("class_name", class_name),
                evt_tag_str("method", "boolean queue(String)"), NULL);
      return FALSE;
  }

  self->dest_impl.mi_flush = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "flush", "()Z");
  if (!self->dest_impl.mi_flush)
    {
      msg_error("Can't find method in class",
                evt_tag_str("class_name", class_name),
                evt_tag_str("method", "boolean flush()"), NULL);
      return FALSE;
    }

  self->dest_impl.dest_object = CALL_JAVA_FUNCTION(java_env, NewObject, self->loaded_class, self->dest_impl.mi_constructor, NULL);
  if (!self->dest_impl.dest_object)
    {
      msg_error("Can't create object",
                evt_tag_str("class_name", class_name),
                NULL);
      return FALSE;
    }
  return TRUE;
}


void
java_destination_proxy_free(JavaDestinationProxy *self)
{
  JNIEnv *env = NULL;
  env = java_machine_get_env(self->java_machine, &env);
  if (self->dest_impl.dest_object)
    {
      CALL_JAVA_FUNCTION(env, DeleteLocalRef, self->dest_impl.dest_object);
    }

  if (self->loaded_class)
    {
      CALL_JAVA_FUNCTION(env, DeleteLocalRef, self->loaded_class);
    }
  java_machine_unref(self->java_machine);
  g_free(self);
}

JavaDestinationProxy *
java_destination_proxy_new(const gchar *class_name, const gchar *class_path)
{
  JavaDestinationProxy *self = g_new0(JavaDestinationProxy, 1);
  self->java_machine = java_machine_ref();
  
  if (!__load_destination_object(self, class_name, class_path))
    {
      goto error;
    }

  return self;
error:
  java_destination_proxy_free(self);
  return NULL;
}

gboolean
java_destination_proxy_queue(JavaDestinationProxy *self, JNIEnv *env, GString *formatted_message)
{
  jstring message = CALL_JAVA_FUNCTION(env, NewStringUTF, formatted_message->str);
  jboolean res = CALL_JAVA_FUNCTION(env, CallBooleanMethod, self->dest_impl.dest_object, self->dest_impl.mi_queue, message);
  CALL_JAVA_FUNCTION(env, DeleteLocalRef, message);
  return !!(res);
}

gboolean
java_destination_proxy_init(JavaDestinationProxy *self, JNIEnv *env, void *ptr)
{
  gboolean result;

  self->syslog_ng_class = syslog_ng_class_new(ptr);
  if (!self->syslog_ng_class)
    {
      msg_error("Failed to create SyslogNg object", NULL);
      goto error;
    }
  result = CALL_JAVA_FUNCTION(env, CallBooleanMethod, self->dest_impl.dest_object, self->dest_impl.mi_init, self->syslog_ng_class->syslogng_object);
  if (!result)
    {
      goto error;
    }
  return TRUE;
error:
   (*env)->ExceptionDescribe(env);
   return FALSE; 
}

void
java_destination_proxy_deinit(JavaDestinationProxy *self, JNIEnv *env)
{
  CALL_JAVA_FUNCTION(env, CallVoidMethod, self->dest_impl.dest_object, self->dest_impl.mi_deinit);
  syslog_ng_class_free(self->syslog_ng_class);
}

gboolean
java_destination_proxy_flush(JavaDestinationProxy *self, JNIEnv *env)
{
  return CALL_JAVA_FUNCTION(env, CallBooleanMethod, self->dest_impl.dest_object, self->dest_impl.mi_flush);
}


