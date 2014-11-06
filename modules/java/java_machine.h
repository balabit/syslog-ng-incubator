#ifndef JAVA_MACHINE_H
#define JAVA_MACHINE_H 1

#include <jni.h>
#include <glib.h>

typedef struct _JavaVMSingleton JavaVMSingleton;

JavaVMSingleton *java_machine_ref();
void java_machine_unref(JavaVMSingleton *self);
gboolean java_machine_start(JavaVMSingleton* self, JNIEnv **env);

void java_machine_attach_thread(JavaVMSingleton* self, JNIEnv **penv);
void java_machine_detach_thread(JavaVMSingleton* self);

#endif
