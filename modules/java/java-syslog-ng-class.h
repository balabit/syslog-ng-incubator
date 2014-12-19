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


#ifndef JAVA_SYSLOG_NG_CLASS_H_
#define JAVA_SYSLOG_NG_CLASS_H_

#include <jni.h>
#include <syslog-ng.h>
#include "java-class-loader.h"

typedef struct _SyslogNgClass {
  jclass syslogng_class;
  jobject syslogng_object;
  jmethodID syslogng_constructor_id;
} SyslogNgClass;

SyslogNgClass *syslog_ng_class_new(JNIEnv *java_env, ClassLoader *loader, gpointer ptr);
void syslog_ng_class_free(SyslogNgClass *self, JNIEnv *java_env);

#endif /* JAVA_SYSLOG_NG_CLASS_H_ */
