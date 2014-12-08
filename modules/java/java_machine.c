#include "java_machine.h"
#include "syslog-ng.h"
#include "atomic.h"

struct _JavaVMSingleton
{
  GAtomicCounter ref_cnt;
  JavaVMOption options[3];
  JNIEnv *env;
  JavaVM *jvm;
  JavaVMInitArgs vm_args;
  GString *class_path;
};

static JavaVMSingleton *g_jvm_s;

JavaVMSingleton *
java_machine_ref()
{
  if (g_jvm_s)
    {
      g_atomic_counter_inc(&g_jvm_s->ref_cnt);
    }
  else
    {
      g_jvm_s = g_new0(JavaVMSingleton, 1);
      g_atomic_counter_set(&g_jvm_s->ref_cnt, 1);

      g_jvm_s->class_path = g_string_new(module_path);
      g_string_append(g_jvm_s->class_path, "/SyslogNg.jar");
    }
  return g_jvm_s;
}

void
java_machine_unref(JavaVMSingleton *self)
{
  g_assert(self == g_jvm_s);
  if (g_atomic_counter_dec_and_test(&self->ref_cnt))
    {
      g_string_free(self->class_path, TRUE);
      if (self->jvm)
        {
          JavaVM jvm = *(self->jvm);
          jvm->DestroyJavaVM(self->jvm);
        }
      g_free(self);
      g_jvm_s = NULL;
    }
}

gboolean
java_machine_start(JavaVMSingleton* self, JNIEnv **env)
{
  g_assert(self == g_jvm_s);
  if (!self->jvm)
    {
      long status;
      self->options[0].optionString = g_strdup_printf(
          "-Djava.class.path=%s", self->class_path->str);

      self->options[1].optionString = g_strdup_printf(
          "-Djava.library.path=%s", module_path);

      self->options[2].optionString = g_strdup("-Xrs");

      self->vm_args.version = JNI_VERSION_1_6;
      self->vm_args.nOptions = 3;
      self->vm_args.options = self->options;
      status = JNI_CreateJavaVM(&self->jvm, (void**) &self->env,
                                &self->vm_args);
      if (status == JNI_ERR) {
          return FALSE;
      }
    }
  *env = self->env;
  return TRUE;
}

void
java_machine_attach_thread(JavaVMSingleton* self, JNIEnv **penv)
{
  g_assert(self == g_jvm_s);
  (*(self->jvm))->AttachCurrentThread(self->jvm, (void **)penv, &self->vm_args);
}

void
java_machine_detach_thread(JavaVMSingleton* self)
{
  g_assert(self == g_jvm_s);
  (*(self->jvm))->DetachCurrentThread(self->jvm);
}
