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


#define CALL_JAVA_FUNCTION(env, function, ...) (*(env))->function(env, __VA_ARGS__)

JavaDestinationProxy *
java_destination_proxy_new(JNIEnv *java_env, const gchar *class_name, const gchar *class_path)
{
  JavaDestinationProxy *self = g_new0(JavaDestinationProxy, 1);
  self->loaded_class = CALL_JAVA_FUNCTION(java_env, FindClass, class_name);
  if (!self->loaded_class) {
      msg_error("Can't find class",
                evt_tag_str("class_name", class_name),
                evt_tag_str("class_path", class_path), NULL);
      goto error;
  }
  self->mi_constructor = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "<init>", "()V");
  if (!self->mi_constructor) {
      msg_error("Can't find default constructor for class",
                evt_tag_str("class_name", class_name), NULL);
      goto error;
  }
  self->mi_init = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "init", "(J)Z");

  if (!self->mi_init) {
      msg_error("Can't find method in class",
                evt_tag_str("class_name", class_name),
                evt_tag_str("method", "boolean init()"), NULL);
      goto error;
  }
  self->mi_deinit = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "deinit", "()V");
  if (!self->mi_deinit) {
      msg_error("Can't find method in class",
                evt_tag_str("class_name", class_name),
                evt_tag_str("method", "void deinit()"), NULL);
      goto error;
  }
  self->mi_queue = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "queue", "(Ljava/lang/String;)Z");
  if (!self->mi_queue) {
      msg_error("Can't find method in class",
                evt_tag_str("class_name", class_name),
                evt_tag_str("method", "boolean queue(String)"), NULL);
      goto error;
  }

  self->mi_flush = CALL_JAVA_FUNCTION(java_env, GetMethodID, self->loaded_class, "flush", "()Z");
  if (!self->mi_queue) {
        msg_error("Can't find method in class",
                  evt_tag_str("class_name", class_name),
                  evt_tag_str("method", "boolean flush()"), NULL);
        goto error;
    }

  self->dest_object = CALL_JAVA_FUNCTION(java_env, NewObject, self->loaded_class, self->mi_constructor);
  if (!self->dest_object)
    {
      msg_error("Failed to create object", NULL);
      goto error;
    }
  return self;
error:
  g_free(self);
  return NULL;
}

gboolean
java_destination_proxy_queue(JavaDestinationProxy *self, JNIEnv *env, GString *formatted_message)
{
  jstring message = CALL_JAVA_FUNCTION(env, NewStringUTF, formatted_message->str);
  jboolean res = CALL_JAVA_FUNCTION(env, CallBooleanMethod, self->dest_object, self->mi_queue, message);
  CALL_JAVA_FUNCTION(env, DeleteLocalRef, message);
  return !!(res);
}

gboolean
java_destination_proxy_init(JavaDestinationProxy *self, JNIEnv *env, void *ptr)
{
  return CALL_JAVA_FUNCTION(env, CallBooleanMethod, self->dest_object, self->mi_init, ptr);
}

void
java_destination_proxy_deinit(JavaDestinationProxy *self, JNIEnv *env)
{
  CALL_JAVA_FUNCTION(env, CallVoidMethod, self->dest_object, self->mi_deinit);
}

gboolean
java_destination_proxy_flush(JavaDestinationProxy *self, JNIEnv *env)
{
  return CALL_JAVA_FUNCTION(env, CallBooleanMethod, self->dest_object, self->mi_flush);
}

void
java_destination_proxy_free(JavaDestinationProxy *self, JNIEnv *env)
{
  if (self->dest_object)
    {
      CALL_JAVA_FUNCTION(env, DeleteLocalRef, self->dest_object);
    }
  if (self->loaded_class)
    {
      CALL_JAVA_FUNCTION(env, DeleteLocalRef, self->loaded_class);
    }
  g_free(self);
}

