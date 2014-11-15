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
#include "messages.h"
#include <string.h>

#define SYSLOG_NG_CLASS_LOADER  "org/syslog_ng/SyslogNgClassLoader"
#define SYSLOG_NG_CLASS         "org/syslog_ng/SyslogNg"
#define SYSLOG_NG_JAR           "SyslogNg.jar"

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

typedef struct _JavaDestinationProxy
{
  jclass loaded_class;
  JavaDestinationImpl dest_impl;

  jclass syslogng_class;
  jobject syslogng_object;
  jmethodID mi_syslogng_constructor;

  jclass syslogng_class_loader;
  jobject loader_object;
  jmethodID loader_constructor;
  jmethodID mi_loadclass;

} JavaDestinationProxy;


static gboolean
__load_class_loader(JavaDestinationProxy *self, JNIEnv *java_env, const gchar *class_loader_name, const gchar *class_path)
{
  self->syslogng_class_loader = CALL_JAVA_FUNCTION(java_env, FindClass, class_loader_name);
  if (!self->syslogng_class_loader)
    {
      msg_error("Can't find class",
                evt_tag_str("class_name", class_loader_name),
                NULL);
      return FALSE;
    }
  self->loader_constructor = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->syslogng_class_loader, "<init>", "(Ljava/lang/String;)V");
  if (!self->loader_constructor)
    {
      msg_error("Can't find constructor for SyslogNgClassLoader", NULL);
    }

  self->mi_loadclass = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->syslogng_class_loader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
//  self->mi_loadclass = CALL_JAVA_FUNCTION(java_env, GetStaticMethodID, self->syslogng_class_loader, "loadClassFromPathList", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Class;");
  if (!self->mi_loadclass)
    {
      msg_error("Can't find static method in class",
                evt_tag_str("class_name", class_loader_name),
                evt_tag_str("method", "Class loadClass(String className)"),
                NULL);
      return FALSE;
    }
  GString *g_class_path = g_string_new(module_path);
  g_string_append(g_class_path, "/" SYSLOG_NG_JAR);
  if (class_path && (strlen(class_path) > 0))
    {
      g_string_append_c(g_class_path, ':');
      g_string_append(g_class_path, class_path);
    }
  jstring str_class_path = CALL_JAVA_FUNCTION(java_env, NewStringUTF, g_class_path->str);
  self->loader_object = CALL_JAVA_FUNCTION(java_env, NewObject, self->syslogng_class_loader, self->loader_constructor, str_class_path);
  if (!self->loader_object)
    {
      msg_error("Can't create SyslogNgClassLoader", NULL);
      return FALSE;
    }
  CALL_JAVA_FUNCTION(java_env, DeleteLocalRef, str_class_path);
  g_string_free(g_class_path, TRUE);
  return TRUE;
}

static jclass
syslog_ng_class_loader_load_class(JavaDestinationProxy *self, JNIEnv *java_env, const gchar *class_name)
{
  jclass result;

  jstring str_class_name = CALL_JAVA_FUNCTION(java_env, NewStringUTF, class_name);
  result = CALL_JAVA_FUNCTION(java_env, CallObjectMethod, self->loader_object, self->mi_loadclass, str_class_name);

  CALL_JAVA_FUNCTION(java_env, DeleteLocalRef, str_class_name);
  return result;
}

static gboolean
__load_destination_object(JavaDestinationProxy *self, JNIEnv *java_env, const gchar *class_name)
{
  self->loaded_class = syslog_ng_class_loader_load_class(self, java_env, class_name);
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

static gboolean
__load_syslog_ng_class(JavaDestinationProxy *self, JNIEnv *java_env)
{
  self->syslogng_class = syslog_ng_class_loader_load_class(self, java_env, "org.syslog_ng.SyslogNg");
  if (!self->syslogng_class)
    {
      (*java_env)->ExceptionDescribe(java_env);
      msg_error("Can't find SyslogNg class", NULL);
      return FALSE;
    }

  self->mi_syslogng_constructor = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->syslogng_class, "<init>", "(J)V");
  if (!self->mi_syslogng_constructor)
    {
      msg_error("Can't find method in class",
                evt_tag_str("class_name", SYSLOG_NG_CLASS),
                evt_tag_str("method", "SyslogNg(long)"),
                NULL);
      return FALSE;
    }
  return TRUE;
}

void
java_destination_proxy_free(JavaDestinationProxy *self, JNIEnv *env)
{
  if (self->dest_impl.dest_object)
    {
      CALL_JAVA_FUNCTION(env, DeleteLocalRef, self->dest_impl.dest_object);
    }
  if (self->syslogng_object)
    {
      CALL_JAVA_FUNCTION(env, DeleteLocalRef, self->syslogng_object);
    }
  if (self->syslogng_class_loader)
    {
      CALL_JAVA_FUNCTION(env, DeleteLocalRef, self->syslogng_class_loader);
    }
  if (self->loaded_class)
    {
      CALL_JAVA_FUNCTION(env, DeleteLocalRef, self->loaded_class);
    }
  if (self->syslogng_class)
    {
      CALL_JAVA_FUNCTION(env, DeleteLocalRef, self->syslogng_class);
    }
  g_free(self);
}

JavaDestinationProxy *
java_destination_proxy_new(JNIEnv *java_env, const gchar *class_name, const gchar *class_path)
{
  JavaDestinationProxy *self = g_new0(JavaDestinationProxy, 1);

  if (!__load_class_loader(self, java_env, SYSLOG_NG_CLASS_LOADER, class_path))
    {
      goto error;
    }
  
  if (!__load_destination_object(self, java_env, class_name))
    {
      goto error;
    }

  if (!__load_syslog_ng_class(self, java_env))
    {
      goto error;
    }
  return self;
error:
  java_destination_proxy_free(self, java_env);
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
  self->syslogng_object = CALL_JAVA_FUNCTION(env,
                                             NewObject,
                                             self->syslogng_class,
                                             self->mi_syslogng_constructor,
                                             ptr);
  if (!self->syslogng_object)
    {
      msg_error("Failed to create SyslogNg object", NULL);
      goto error;
    }
  result = CALL_JAVA_FUNCTION(env, CallBooleanMethod, self->dest_impl.dest_object, self->dest_impl.mi_init, self->syslogng_object);
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
}

gboolean
java_destination_proxy_flush(JavaDestinationProxy *self, JNIEnv *env)
{
  return CALL_JAVA_FUNCTION(env, CallBooleanMethod, self->dest_impl.dest_object, self->dest_impl.mi_flush);
}


