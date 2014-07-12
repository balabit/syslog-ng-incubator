/*
 * Copyright (c) 2014 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2014 Laszlo Meszaros <lmesz@balabit.hu>
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

#include "java-destination.h"
#include "messages.h"
#include "stats.h"
#include "logqueue.h"
#include "driver.h"

void java_dd_start_watches(JavaDestDriver *self);

static void
java_dd_queue(LogPipe *s, LogMessage *msg, const LogPathOptions *path_options, gpointer user_data)
{
	JavaDestDriver *self = (JavaDestDriver *)s;
	log_queue_push_tail(self->log_queue, msg, path_options);
}

static gboolean
java_dd_init_jvm(JavaDestDriver *self)
{
	long status;
	self->options[0].optionString = g_strdup_printf("-Djava.class.path=%s", self->class_path);

	self->vm_args.version = JNI_VERSION_1_6;
	self->vm_args.nOptions = 1;
	self->vm_args.options = self->options;
	status = JNI_CreateJavaVM(&self->jvm, (void**)&self->env, &self->vm_args);
	if (status == JNI_ERR)
	{
		return FALSE;
	}
	return TRUE;
}

gboolean java_dd_load_class(JavaDestDriver *self) {
	JNIEnv env = *(self->env);
	self->loaded_class = (*(self->env))->FindClass(self->env, self->class_name);
	if (!self->loaded_class) {
		msg_error("Can't find class",
				evt_tag_str("class_name", self->class_name),
				evt_tag_str("class_path", self->class_path), NULL);
		return FALSE;
	}
	self->mi_constructor = env->GetMethodID(self->env, self->loaded_class,
			"<init>", "()V");
	if (!self->mi_constructor) {
		msg_error("Can't find default constructor for class",
				evt_tag_str("class_name", self->class_name), NULL);
		return FALSE;
	}
	self->mi_init = env->GetMethodID(self->env, self->loaded_class, "init", "()Z");
	if (!self->mi_init) {
		msg_error("Can't find method in class",
				evt_tag_str("class_name", self->class_name),
				evt_tag_str("method", "boolean init()"), NULL);
		return FALSE;
	}
	self->mi_deinit = env->GetMethodID(self->env, self->loaded_class, "deinit",
			"()V");
	if (!self->mi_deinit) {
		msg_error("Can't find method in class",
				evt_tag_str("class_name", self->class_name),
				evt_tag_str("method", "void deinit()"), NULL);
		return FALSE;
	}
	self->mi_queue = env->GetMethodID(self->env, self->loaded_class, "queue",
			"(Ljava/lang/String;)Z");
	if (!self->mi_queue) {
		msg_error("Can't find method in class",
				evt_tag_str("class_name", self->class_name),
				evt_tag_str("method", "boolean queue(String)"), NULL);
		return FALSE;
	}
	self->dest_object = env->NewObject(self->env, self->loaded_class, self->mi_constructor);
	if (!self->dest_object)
	{
		msg_error("Failed to create object", NULL);
		return FALSE;
	}
	return TRUE;
}

gboolean
java_dd_init_dest_object(JavaDestDriver *self)
{
	JNIEnv env = *(self->env);
	return env->CallBooleanMethod(self->env, self->dest_object, self->mi_init);
}

void
java_dd_deinit_dest_object(JavaDestDriver *self)
{
	JNIEnv env = *(self->env);
	env->CallVoidMethod(self->env, self->dest_object, self->mi_deinit);
}

void
java_dd_set_class_path(LogDriver *s, const gchar *class_path)
{
	JavaDestDriver *self = (JavaDestDriver *)s;
	g_free(self->class_path);
	self->class_path = g_strdup(class_path);
}

void
java_dd_set_class_name(LogDriver *s, const gchar *class_name)
{
	JavaDestDriver *self = (JavaDestDriver *)s;
	g_free(self->class_name);
	self->class_name = g_strdup(class_name);
}

void
java_dd_set_template_string(LogDriver *s, const gchar *template_string)
{
	JavaDestDriver *self = (JavaDestDriver *)s;
	g_free(self->template_string);
	self->template_string = g_strdup(template_string);
}

gboolean
java_dd_init(LogPipe *s)
{
	JavaDestDriver *self = (JavaDestDriver *)s;
	GError *error = NULL;
	if(!log_dest_driver_init_method(s))
		return FALSE;
	if (!log_template_compile(self->template, self->template_string, &error))
	{
		msg_error("Can't compile template",
					evt_tag_str("template", self->template_string),
					evt_tag_str("error", error->message),
					NULL
				);
		return FALSE;
	}
	if(!java_dd_init_jvm(self))
		return FALSE;
	if(!java_dd_load_class(self))
		return FALSE;
	self->log_queue = log_dest_driver_acquire_queue(&self->super, "testjava");
	java_dd_start_watches(self);
	return java_dd_init_dest_object(self);
}

gboolean
java_dd_deinit(LogPipe *s)
{
	JavaDestDriver *self = (JavaDestDriver *)s;
	java_dd_deinit_dest_object(self);
	return TRUE;
}

void
java_dd_free(LogPipe *s)
{
	JavaDestDriver *self = (JavaDestDriver *)s;
	if (self->dest_object)
	{
		JNIEnv env = *(self->env);
		env->DeleteLocalRef(self->env, self->dest_object);
	}
	if(self->loaded_class)
	{
		JNIEnv env = *(self->env);
		env->DeleteLocalRef(self->env, self->loaded_class);
	}

	if(self->jvm)
	{
		JavaVM jvm = *(self->jvm);
		jvm->DestroyJavaVM(self->jvm);
	}
	g_free(self->class_name);
	g_free(self->class_path);
}
void
java_dd_wake_up(gpointer user_data)
{
	JavaDestDriver *self = (JavaDestDriver *)user_data;
	iv_event_post(&self->wake_up_event);
}

void
java_dd_stop_watches(JavaDestDriver *self)
{
	log_queue_reset_parallel_push(self->log_queue);
	if(iv_task_registered(&self->immed_io_task))
	{
		iv_task_unregister(&self->immed_io_task);
	}
}

void
java_dd_attach_env(JavaDestDriver *self, JNIEnv **penv)
{
	(*(self->jvm))->AttachCurrentThread(self->jvm, (void **)penv, &self->vm_args);
}

void
java_dd_detach_env(JavaDestDriver *self)
{
	(*(self->jvm))->DetachCurrentThread(self->jvm);
}

gboolean
java_dd_send_to_object(JavaDestDriver *self, LogMessage *msg, JNIEnv *env)
{
	log_template_format(self->template, msg, NULL, LTZ_LOCAL, 0, NULL, self->formatted_message);
	jstring message = (*env)->NewStringUTF(env, self->formatted_message->str);
	jboolean res = (*env)->CallBooleanMethod(env, self->dest_object, self->mi_queue, message);
	(*env)->DeleteLocalRef(env, message);
	return !!(res);
}

void
java_dd_work_perform(gpointer data)
{
	JavaDestDriver *self = (JavaDestDriver *)data;
	gboolean sent = TRUE;
	JNIEnv *env = NULL;
	java_dd_attach_env(self, &env);
	while (sent && !main_loop_io_worker_job_quit())
	{
		LogMessage *lm;
		LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;
		gboolean consumed = FALSE;

		if (!log_queue_pop_head(self->log_queue, &lm, &path_options, TRUE, TRUE))
		{
			/* no more items are available */
			break;
		}

		log_msg_refcache_start_consumer(lm, &path_options);
		msg_set_context(lm);
		sent = java_dd_send_to_object(self, lm, env);
		if (sent)
			log_queue_ack_backlog(self->log_queue, 1);
		else
			log_queue_rewind_backlog(self->log_queue);
		msg_set_context(NULL);
		log_msg_refcache_stop();
	}
	java_dd_detach_env(self);
}

void
java_dd_work_finished(gpointer data)
{
	JavaDestDriver *self = (JavaDestDriver *)data;
	if (self->super.super.super.flags & PIF_INITIALIZED)
	{
		java_dd_start_watches(self);
	}
	log_pipe_unref(&self->super.super.super);
}

void
java_dd_process_output(JavaDestDriver *self)
{
	main_loop_assert_main_thread();

	java_dd_stop_watches(self);
	log_pipe_ref(&self->super.super.super);
	if (self->threaded)
	{
		main_loop_io_worker_job_submit(&self->io_job);
	}
	else
	{
		/* Checking main_loop_io_worker_job_quit() helps to speed up the
		 * reload process.  If reload/shutdown is requested we shouldn't do
		 * anything here, a final flush will be attempted in
		 * log_writer_deinit().
		 *
		 * Our current understanding is that it doesn't prevent race
		 * conditions of any kind.
		 */

		if (!main_loop_io_worker_job_quit())
		{
			java_dd_work_perform(self);
			java_dd_work_finished(self);
		}
	}

}

void
java_dd_update_watches(gpointer cookie)
{
	JavaDestDriver *self = (JavaDestDriver *)cookie;
	if (log_queue_check_items(self->log_queue, NULL, java_dd_wake_up, self, NULL))
	{
		java_dd_process_output(self);
	}
}

void
java_dd_start_watches(JavaDestDriver *self)
{
	java_dd_update_watches(self);
}

void
java_dd_init_watches(JavaDestDriver *self)
{
  IV_EVENT_INIT(&self->wake_up_event);
  self->wake_up_event.cookie = self;
  self->wake_up_event.handler = java_dd_update_watches;
  iv_event_register(&self->wake_up_event);

  IV_TASK_INIT(&self->immed_io_task);
  self->immed_io_task.cookie = self;
  self->immed_io_task.handler = java_dd_update_watches;

  main_loop_io_worker_job_init(&self->io_job);
  self->io_job.user_data = self;
  self->io_job.work = (void (*)(void *)) java_dd_work_perform;
  self->io_job.completion = (void (*)(void *)) java_dd_work_finished;
}

LogDriver *
java_dd_new(GlobalConfig *cfg)
{
  JavaDestDriver *self = g_new0(JavaDestDriver, 1);

  log_dest_driver_init_instance(&self->super);
  self->super.super.super.free_fn = java_dd_free;
  self->super.super.super.init = java_dd_init;
  self->super.super.super.deinit = java_dd_deinit;
  self->super.super.super.queue = java_dd_queue;

  self->template = log_template_new(cfg, "java_dd_template");
  java_dd_set_class_name(&self->super.super, "TestClass");
  java_dd_set_class_path(&self->super.super, ".");
  java_dd_set_template_string(&self->super.super, "$ISODATE $HOST $MSGHDR$MSG");
  self->threaded = cfg->threaded;
  self->formatted_message = g_string_sized_new(1024);
  java_dd_init_watches(self);
  return (LogDriver *)self;
}
