/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Gyula Petrovics <pgyula@balabit.hu>
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

#include "mysql-mod.h"

#include "logqueue.h"
#include "templates.h"
#include "messages.h"
#include "misc.h"
#include "stats.h"
#include "apphook.h"
#include "timeutils.h"

#include <string.h>
#include <mysql.h>

/* field flags */
enum
{
  MYSQL_FF_DEFAULT = 0x0001,
};

/* destination driver flags */
enum
{
  MYSQL_DDF_EXPLICIT_COMMITS = 0x0001,
  MYSQL_DDF_DONT_CREATE_TABLES = 0x0002,
};

typedef struct
{
  guint32 flags;
  gchar *name;
  gchar *type;
  LogTemplate *value;
} MySQLField;

//typedef void * mysql_result;

typedef struct
{
  LogDestDriver super;

  /* read by the db thread */
  gchar *host;
  gint port;
  gchar *user;
  gchar *password;
  gchar *database;
  gchar *encoding;
  GList *columns;
  GList *values;
  GList *indexes;
  LogTemplate *table;
  gint fields_len;
  MySQLField *fields;
  gchar *null_value;
  gint time_reopen;
  gint num_retries;
  gint flush_lines;
  gint flush_timeout;
  gint flush_lines_queued;
  gint flags;
  GList *session_statements;

  LogTemplateOptions template_options;

  StatsCounterItem *dropped_messages;
  StatsCounterItem *stored_messages;

  /* shared by the main/db thread */
  GThread *db_thread;
  GMutex *db_thread_mutex;
  GCond *db_thread_wakeup_cond;
  gboolean db_thread_terminate;
  gboolean db_thread_suspended;
  GTimeVal db_thread_suspend_target;
  LogQueue *queue;
  /* used exclusively by the db thread */
  gint32 seq_num;
  GHashTable *validated_tables;
  guint32 failed_message_counter;

  MYSQL *mysql;

  gint bulk_insert_index;
  gboolean bulk_insert;
  GString *bulk_insert_query;
} MySQLDestDriver;


#define MAX_FAILED_ATTEMPTS 3

void
mysql_dd_set_host(LogDriver *s, const gchar *host)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  g_free(self->host);
  self->host = g_strdup(host);
}

void
mysql_dd_set_bulk_insert(LogDriver *s, gboolean bulk_insert)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  self->bulk_insert = bulk_insert;

  // FIXME
  self->bulk_insert_index = 0;
  self->bulk_insert_query  = g_string_new(NULL);
}

void
mysql_dd_set_port(LogDriver *s, gint port)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  self -> port = port;
}

void
mysql_dd_set_user(LogDriver *s, const gchar *user)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  g_free(self->user);
  self->user = g_strdup(user);
}

void
mysql_dd_set_password(LogDriver *s, const gchar *password)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  g_free(self->password);
  self->password = g_strdup(password);
}

void
mysql_dd_set_database(LogDriver *s, const gchar *database)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  g_free(self->database);
  self->database = g_strdup(database);
}

void
mysql_dd_set_table(LogDriver *s, const gchar *table)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  // FIXME: use template_content
  log_template_compile(self->table, table, NULL);
}

void
mysql_dd_set_columns(LogDriver *s, GList *columns)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  string_list_free(self->columns);
  self->columns = columns;
}

void
mysql_dd_set_indexes(LogDriver *s, GList *indexes)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  string_list_free(self->indexes);
  self->indexes = indexes;
}

// FIXME: use template list here
void
mysql_dd_set_values(LogDriver *s, GList *values)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  string_list_free(self->values);
  self->values = values;
}

void
mysql_dd_set_null_value(LogDriver *s, const gchar *null)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  g_free(self->null_value);
  self->null_value = g_strdup(null);
}

void
mysql_dd_set_retries(LogDriver *s, gint num_retries)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  self->num_retries = (num_retries < 1) ? 1 : num_retries;
}

void
mysql_dd_set_flush_lines(LogDriver *s, gint flush_lines)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  self->flush_lines = flush_lines;
}

void
mysql_dd_set_flush_timeout(LogDriver *s, gint flush_timeout)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  self->flush_timeout = flush_timeout;
}

void
mysql_dd_set_session_statements(LogDriver *s, GList *session_statements)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  self->session_statements = session_statements;
}

/**
 * Private stuff
 **/

static gboolean
mysql_dd_run_query(MySQLDestDriver *self, const gchar *query)
{
  const gchar *db_error;

  msg_debug("Running MYSQL query",
            evt_tag_str("driver", self->super.super.id),
            evt_tag_str("query", query),
            NULL);

  if (mysql_query(self->mysql, query))
    {
      db_error = mysql_error(self->mysql);
      msg_error("Error running MYSQL query",
                evt_tag_str("driver", self->super.super.id),
                evt_tag_str("host", self->host),
                evt_tag_int("port", self->port),
                evt_tag_str("user", self->user),
                evt_tag_str("database", self->database),
                evt_tag_str("error", db_error),
                evt_tag_str("query", query),
                NULL);
      return FALSE;
    }

  return TRUE;
}

static gboolean
mysql_dd_check_sql_identifier(gchar *token, gboolean sanitize)
{
  gint i;

  for (i = 0; token[i]; i++)
    {
      if (!((token[i] == '.') || (token[i] == '_') ||
            (i && token[i] >= '0' && token[i] <= '9') ||
            (g_ascii_tolower(token[i]) >= 'a' && g_ascii_tolower(token[i]) <= 'z')))
        {
          if (sanitize)
            token[i] = '_';
          else
            return FALSE;
        }
    }
  return TRUE;
}

static gboolean
mysql_dd_create_index(MySQLDestDriver *self, gchar *table, gchar *column)
{
  GString *query_string;
  gboolean success = TRUE;

  query_string = g_string_sized_new(64);

  g_string_printf(query_string, "CREATE INDEX %s_%s_idx ON %s (%s)",
                  table, column, table, column);

  if (!mysql_dd_run_query(self, query_string->str))
    {
      msg_error("Error adding missing index",
                evt_tag_str("driver", self->super.super.id),
                evt_tag_str("table", table),
                evt_tag_str("column", column),
                NULL);
      success = FALSE;
    }

  g_string_free(query_string, TRUE);
  return success;
}

static GString *
mysql_dd_validate_table(MySQLDestDriver *self, LogMessage *msg)
{
  GString *query_string, *table;
  gint i;

  table = g_string_new(NULL);
  log_template_format(self->table, msg, &self->template_options, LTZ_LOCAL, 0, NULL, table);

  /*
  if (self->flags & AFMYSQL_DDF_DONT_CREATE_TABLES)
    return table;
  */

  mysql_dd_check_sql_identifier(table->str, TRUE);

  if (g_hash_table_lookup(self->validated_tables, table->str))
    return table;

  query_string = g_string_new(NULL);
  g_string_printf(query_string, "CREATE TABLE IF NOT EXISTS %s.%s(", self -> user, table -> str);
  for (i = 0; i < self->fields_len; i++)
        {
          g_string_append_printf(query_string, "%s %s", self->fields[i].name, self->fields[i].type);
          if (i != self->fields_len - 1)
            g_string_append(query_string, ", ");
        }
  g_string_append(query_string, ");");
  if (!mysql_dd_run_query( self, query_string -> str))
    {
       msg_error("Error creating table, giving up",
                 evt_tag_str("driver", self->super.super.id),
                 evt_tag_str("table", table->str),
                 NULL);
    }
  g_hash_table_insert(self->validated_tables, g_strdup(table->str), GUINT_TO_POINTER(TRUE));
  g_string_free(query_string, TRUE);

  return table;
}

static gboolean
mysql_dd_begin_txn(MySQLDestDriver *self)
{
  if (!mysql_dd_run_query(self, "BEGIN;"))
    return FALSE;
  return TRUE;
}

static gboolean
mysql_dd_commit_txn(MySQLDestDriver *self)
{
  gboolean success;

  success = mysql_dd_run_query(self, "COMMIT;");

  if (!success)
    {
      msg_notice("MYSQL transaction commit failed, rewinding backlog and starting again",
                 NULL);
      log_queue_rewind_backlog(self->queue);

      goto out;
    }

  log_queue_ack_backlog(self->queue, self->flush_lines_queued);

 out:
  self->flush_lines_queued = 0;
  return success;
}

static void
mysql_dd_suspend(MySQLDestDriver *self)
{
  self->db_thread_suspended = TRUE;
  g_get_current_time(&self->db_thread_suspend_target);
  g_time_val_add(&self->db_thread_suspend_target, self->time_reopen * 1000 * 1000); /* the timeout expects microseconds */
}

static void
mysql_dd_disconnect(MySQLDestDriver *self)
{
  mysql_close(self->mysql);
  msg_debug("MYSQL disconnected",
            evt_tag_str("driver", self->super.super.id),
            NULL);
}

static gboolean
mysql_dd_connect(MySQLDestDriver *self)
{
  if (self->mysql)
    return TRUE;

  self->mysql = mysql_init(NULL);
  if(!self->mysql)
    {
      msg_error("No such mysql driver",
                evt_tag_str("error", mysql_error(self -> mysql)),
                NULL);
      return FALSE;
    }

  if (!mysql_real_connect(self->mysql, self->host, self->user, self->password,
                          self->database, self->port, NULL, 0))
    {
      msg_error("Error establishing MYSQL connection",
                evt_tag_str("driver", self->super.super.id),
                evt_tag_str("error", mysql_error(self->mysql)),
                NULL);
      return FALSE;
    }

  if (self->session_statements != NULL)
    {
      GList *l;

      for (l = self->session_statements; l; l = l->next)
        {
          if (!mysql_dd_run_query(self, (gchar *) l->data))
            {
              msg_error("Error executing SQL connection statement",
                        evt_tag_str("driver", self->super.super.id),
                        evt_tag_str("statement", (gchar *) l->data),
                        NULL);
              return FALSE;
            }
        }
    }

  return TRUE;
}

static gboolean
mysql_dd_insert_fail_handler(MySQLDestDriver *self, LogMessage *msg,
                             LogPathOptions *path_options)
{
  if (self->failed_message_counter < self->num_retries - 1)
    {
      log_queue_push_head(self->queue, msg, path_options);

      /* database connection status sanity check after failed query */
      if (mysql_ping(self->mysql) != 0)
        {
          msg_error("Error, no SQL connection after failed query attempt",
                    evt_tag_str("driver", self->super.super.id),
                    evt_tag_str("error", mysql_error(self -> mysql)),
                    NULL);
          return FALSE;
        }

      self->failed_message_counter++;
      return FALSE;
    }
  msg_error("Multiple failures while inserting this record into the database, message dropped",
            evt_tag_str("driver", self->super.super.id),
            evt_tag_int("attempts", self->num_retries),
            NULL);

  stats_counter_inc(self->dropped_messages);
  log_msg_drop(msg, path_options);
  self->failed_message_counter = 0;

  return TRUE;
}

static GString *
mysql_dd_construct_query(MySQLDestDriver *self, GString *table,
                         LogMessage *msg)
{
  GString *value;
  GString *query_string;
  gint i;

  value = g_string_new(NULL);
  query_string = g_string_new(NULL);

    if (self->bulk_insert_index == 0)
    {
      g_string_printf(query_string, "INSERT INTO %s.%s (", self -> user, table->str);
      for (i = 0; i < self->fields_len; i++)
        {
          g_string_append(query_string, self->fields[i].name);
          if (i != self->fields_len - 1)
            g_string_append(query_string, ", ");
        }
      g_string_append(query_string, ") VALUES (");
    }
  else
    g_string_append(query_string, ", (");

  for (i = 0; i < self->fields_len; i++)
    {

      if (self->fields[i].value == NULL)
        {
          /* the config used the 'default' value for this column -> the fields[i].value is NULL, use SQL default */
          g_string_append(query_string, "DEFAULT");
        }
      else
        {
          log_template_format(self->fields[i].value, msg, &self->template_options, LTZ_SEND, self->seq_num, NULL, value);

          if (self->null_value && strcmp(self->null_value, value->str) == 0)
            {
              g_string_append(query_string, "NULL");
            }
          else
            {
              gint escaped_msg_len = strlen(value->str)*2+1;
              char *escaped_msg=g_new0(char, escaped_msg_len);
              mysql_real_escape_string(self->mysql, escaped_msg, value->str, strlen(value->str));
              if (escaped_msg)
                {
                  g_string_append(query_string, "'");
                  g_string_append(query_string, escaped_msg);
                  g_string_append(query_string, "'");
                  g_free(escaped_msg);
                }
              else
                {
                  g_string_append(query_string, "''");
                }
            }
        }

      if (i != self->fields_len - 1)
        g_string_append(query_string, ", ");
    }
  g_string_append(query_string, ")");

  msg_debug("Constructed MySQL query",
            evt_tag_str("driver", self->super.super.id),
            evt_tag_str("query", query_string->str),
            NULL);

  g_string_free(value, TRUE);

  return query_string;
}

static gboolean
mysql_dd_insert_db(MySQLDestDriver *self)
{
  GString *table, *query_string;
  LogMessage *msg;
  gboolean success;
  LogPathOptions path_options = LOG_PATH_OPTIONS_INIT;

  mysql_dd_connect(self);

  success = log_queue_pop_head(self->queue, &msg, &path_options, TRUE, FALSE);
  if (!success)
    return TRUE;

  msg_set_context(msg);

  table = mysql_dd_validate_table(self, msg);
  if (!table)
    {
      // If validate table is FALSE then close the connection and wait time_reopen time (next call)
      msg_error("Error checking table, disconnecting from database, trying again shortly",
                evt_tag_str("driver", self->super.super.id),
                evt_tag_int("time_reopen", self->time_reopen),
                NULL);
      msg_set_context(NULL);
      g_string_free(table, TRUE);
      return mysql_dd_insert_fail_handler(self, msg, &path_options);
    }

  query_string = mysql_dd_construct_query(self, table, msg);

  if (self->flush_lines_queued == 0 && !mysql_dd_begin_txn(self))
    return FALSE;

  if (!self->bulk_insert)
    success = mysql_dd_run_query(self, query_string->str);
  else
    {
      if (self->bulk_insert_index < self->flush_lines)
        {
          g_string_append(self->bulk_insert_query, query_string->str);
          self->bulk_insert_index++;
        }

      if (self->bulk_insert_index >= self->flush_lines)
        {
          success = mysql_dd_run_query(self, self->bulk_insert_query->str);
          self->bulk_insert_index = 0;
          g_string_truncate(self->bulk_insert_query, 0);
        }
    }

  if (success && self->flush_lines_queued != -1)
    {
      self->flush_lines_queued++;
      if (self->flush_lines && self->flush_lines_queued == self->flush_lines && !mysql_dd_commit_txn(self))
        return FALSE;
    }
  g_string_free(table, TRUE);
  g_string_free(query_string, TRUE);

  msg_set_context(NULL);

  if (!success)
    return mysql_dd_insert_fail_handler(self, msg, &path_options);

  log_msg_ack(msg, &path_options);
  log_msg_unref(msg);
  step_sequence_number(&self->seq_num);
  self->failed_message_counter = 0;

  return TRUE;
}


static void
mysql_dd_message_became_available_in_the_queue(gpointer user_data)
{
  MySQLDestDriver *self = (MySQLDestDriver *) user_data;

  g_mutex_lock(self->db_thread_mutex);
  g_cond_signal(self->db_thread_wakeup_cond);
  g_mutex_unlock(self->db_thread_mutex);
}

static void
mysql_dd_wait_for_suspension_wakeup(MySQLDestDriver *self)
{
 if (!self->db_thread_terminate)
    g_cond_timed_wait(self->db_thread_wakeup_cond, self->db_thread_mutex, &self->db_thread_suspend_target);
  self->db_thread_suspended = FALSE;
}

static gpointer
mysql_dd_database_thread(gpointer arg)
{
  MySQLDestDriver *self = (MySQLDestDriver *) arg;

  msg_verbose("Database thread started",
              evt_tag_str("driver", self->super.super.id),
              NULL);

  while (!self->db_thread_terminate)
    {
      g_mutex_lock(self->db_thread_mutex);
      if (self->db_thread_suspended)
        {
          mysql_dd_wait_for_suspension_wakeup(self);
        }
      else if (!log_queue_check_items(self->queue, NULL, mysql_dd_message_became_available_in_the_queue, self, NULL))
        {
          if (self->flush_lines_queued > 0)
            {
              if (!mysql_dd_commit_txn(self))
                {
                  mysql_dd_disconnect(self);
                  mysql_dd_suspend(self);
                  g_mutex_unlock(self->db_thread_mutex);
                  continue;
                }
            }
          else if (!self->db_thread_terminate)
            {
              g_cond_wait(self->db_thread_wakeup_cond, self->db_thread_mutex);
            }

          /* we loop back to check if the thread was requested to terminate */
        }
      g_mutex_unlock(self->db_thread_mutex);

      if (self->db_thread_terminate)
        break;

      if (!mysql_dd_insert_db(self))
        {
          mysql_dd_disconnect(self);
          mysql_dd_suspend(self);
        }
    }

  while (log_queue_get_length(self->queue) > 0)
    {
      if (!mysql_dd_insert_db(self))
        goto exit;
    }

  if (self->flush_lines_queued > 0)
    mysql_dd_commit_txn(self);

 exit:
  mysql_dd_disconnect(self);

  msg_verbose("Database thread finished",
              evt_tag_str("driver", self->super.super.id),
              NULL);
  return NULL;
}

static void
mysql_dd_start_thread(MySQLDestDriver *self)
{
  self->db_thread = create_worker_thread(mysql_dd_database_thread, self, TRUE, NULL);
}

static void
mysql_dd_stop_thread(MySQLDestDriver *self)
{
  g_mutex_lock(self->db_thread_mutex);
  self->db_thread_terminate = TRUE;
  g_cond_signal(self->db_thread_wakeup_cond);
  g_mutex_unlock(self->db_thread_mutex);
  g_thread_join(self->db_thread);
}

static gchar *
mysql_dd_format_stats_instance(MySQLDestDriver *self)
{
  static gchar persist_name[512];

  g_snprintf(persist_name, sizeof(persist_name),
             "mysql,%s,%d,%s,%s",
             self->host, self->port, self->database, self->table->template);
  return persist_name;
}

static inline gchar *
mysql_dd_format_persist_name(MySQLDestDriver *self)
{
  static gchar persist_name[512];

  g_snprintf(persist_name, sizeof(persist_name),
             "mysql_dd(%s,%d,%s,%s)",
             self->host, self->port, self->database, self->table->template);
  return persist_name;
}


static gboolean
mysql_dd_init(LogPipe *s)
{
  MySQLDestDriver *self = (MySQLDestDriver *)s;
  GlobalConfig *cfg = log_pipe_get_config(s);

  gint len_cols, len_values;

  if (!log_dest_driver_init_method(s))
    return FALSE;

  if (!self->columns || !self->values)
    {
      msg_error("Default columns and values must be specified for database destinations",
                evt_tag_str("driver", self->super.super.id),
                NULL);
      return FALSE;
    }

  self->queue = log_dest_driver_acquire_queue(&self->super, mysql_dd_format_persist_name(self));
  log_queue_set_counters(self->queue, self->stored_messages, self->dropped_messages);

  if (!self->fields)
    {
      GList *col, *value;
      gint i;

      len_cols = g_list_length(self->columns);
      len_values = g_list_length(self->values);
      if (len_cols != len_values)
        {
          msg_error("The number of columns and values do not match",
                    evt_tag_str("driver", self->super.super.id),
                    evt_tag_int("len_columns", len_cols),
                    evt_tag_int("len_values", len_values),
                    NULL);
          goto error;
        }

      self->fields_len = len_cols;
      self->fields = g_new0(MySQLField, len_cols);
      for (i = 0, col = self->columns, value = self->values;
           col && value;
           i++, col = col->next, value = value->next)
        {
          gchar *space;

          space = strchr(col->data, ' ');
          if (space)
            {
              self->fields[i].name = g_strndup(col->data, space - (gchar *) col->data);
              while (*space == ' ')
                space++;
              if (*space != '\0')
                self->fields[i].type = g_strdup(space);
              else
                self->fields[i].type = g_strdup("text");
            }
          else
            {
              self->fields[i].name = g_strdup(col->data);
              self->fields[i].type = g_strdup("text");
            }

          if (!mysql_dd_check_sql_identifier(self->fields[i].name, FALSE))
            {
              msg_error("Column name is not a proper MYSQL name",
                        evt_tag_str("driver", self->super.super.id),
                        evt_tag_str("column", self->fields[i].name),
                        NULL);
              return FALSE;
            }

          if (GPOINTER_TO_UINT(value->data) > 4096)
            {
              self->fields[i].value = log_template_new(cfg, NULL);
              log_template_compile(self->fields[i].value, (gchar *) value->data, NULL);
            }
         else
            {
              switch (GPOINTER_TO_UINT(value->data))
                {
                case MYSQL_COLUMN_DEFAULT:
                  self->fields[i].flags |= MYSQL_FF_DEFAULT;
                  break;
                default:
                  g_assert_not_reached();
                  break;
                }
            }
        }
    }

  self->time_reopen = cfg->time_reopen;
  log_template_options_init(&self->template_options, cfg);

  if (self->flush_lines == -1 && cfg->flush_lines != 0)
    self->flush_lines = cfg->flush_lines;
  if (self->flush_timeout == -1)
    self->flush_timeout = cfg->flush_timeout;
  if ((self->flush_lines > 0 || self->flush_timeout > 0))
    self->flush_lines_queued = 0;

  mysql_dd_start_thread(self);
  return TRUE;

 error:

  stats_lock();
  stats_unregister_counter(SCS_SQL | SCS_DESTINATION, self->super.super.id, mysql_dd_format_stats_instance(self), SC_TYPE_STORED, &self->stored_messages);
  stats_unregister_counter(SCS_SQL | SCS_DESTINATION, self->super.super.id, mysql_dd_format_stats_instance(self), SC_TYPE_DROPPED, &self->dropped_messages);
  stats_unlock();

  return FALSE;
}

static gboolean
mysql_dd_deinit(LogPipe *s)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;

  mysql_dd_stop_thread(self);
  log_queue_reset_parallel_push(self->queue);

  log_queue_set_counters(self->queue, NULL, NULL);

  stats_lock();
  stats_unregister_counter(SCS_SQL | SCS_DESTINATION, self->super.super.id, mysql_dd_format_stats_instance(self), SC_TYPE_STORED, &self->stored_messages);
  stats_unregister_counter(SCS_SQL | SCS_DESTINATION, self->super.super.id, mysql_dd_format_stats_instance(self), SC_TYPE_DROPPED, &self->dropped_messages);
  stats_unlock();

  if (!log_dest_driver_deinit_method(s))
    return FALSE;

  return TRUE;
}

static void
mysql_dd_queue(LogPipe *s, LogMessage *msg, const LogPathOptions *path_options, gpointer user_data)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;
  LogPathOptions local_options;

  if (!path_options->flow_control_requested)
    path_options = log_msg_break_ack(msg, path_options, &local_options);

  log_msg_add_ack(msg, path_options);
  log_queue_push_tail(self->queue, log_msg_ref(msg), path_options);
  log_dest_driver_queue_method(s, msg, path_options, user_data);
}

static void
mysql_dd_free(LogPipe *s)
{
  MySQLDestDriver *self = (MySQLDestDriver *) s;
  gint i;

  log_template_options_destroy(&self->template_options);
  if (self->queue)
    log_queue_unref(self->queue);

  for (i = 0; i < self->fields_len; i++)
    {
      g_free(self->fields[i].name);
      g_free(self->fields[i].type);
      log_template_unref(self->fields[i].value);
    }

  g_free(self->fields);
  g_free(self->host);
  g_free(self->user);
  g_free(self->password);
  g_free(self->database);
  g_free(self->encoding);
  if (self->null_value)
    g_free(self->null_value);
  string_list_free(self->columns);
  string_list_free(self->indexes);
  string_list_free(self->values);
  log_template_unref(self->table);
  g_hash_table_destroy(self->validated_tables);

  if (self->session_statements)
    string_list_free(self->session_statements);
  g_mutex_free(self->db_thread_mutex);
  g_cond_free(self->db_thread_wakeup_cond);
  log_dest_driver_free(s);
}

LogDriver *
mysql_dd_new(void)
{
  MySQLDestDriver *self = g_new0(MySQLDestDriver, 1);

  log_dest_driver_init_instance(&self->super);
  self->super.super.super.init = mysql_dd_init;
  self->super.super.super.deinit = mysql_dd_deinit;
  self->super.super.super.queue = mysql_dd_queue;
  self->super.super.super.free_fn = mysql_dd_free;

  //self->type = g_strdup("mysql");
  /*self->host = g_strdup("127.0.0.1");
  self->port = g_strdup("3306");
  self->user = g_strdup("syslog");
  self->password = g_strdup("secret");
  self->database = g_strdup("syslog");*/
  self->encoding = g_strdup("UTF-8");

  self->table = log_template_new(configuration, NULL);
  log_template_compile(self->table, "messages", NULL);
  self->failed_message_counter = 0;

  self->flush_lines = -1;
  self->flush_timeout = -1;
  self->flush_lines_queued = -1;
  self->session_statements = NULL;
  //self->num_retries = MAX_FAILED_ATTEMPTS;

  self->validated_tables = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

  log_template_options_defaults(&self->template_options);
  init_sequence_number(&self->seq_num);

  self->db_thread_wakeup_cond = g_cond_new();
  self->db_thread_mutex = g_mutex_new();

  return &self->super.super;
}
