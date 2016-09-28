/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Gergely Nagy <algernon@balabit.hu>
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

#include "grok-parser.h"
#include <grok.h>
#include <grok_pattern.h>
#include "scratch-buffers.h"
#include "string-list.h"

#define KEY_BUFFER_LENGTH 1024

struct _GrokInstance
{
  grok_t *grok;
  char *grok_pattern;
  char *key_prefix;
  int key_prefix_len;
  GList *tags;
}; 

typedef struct _GrokPattern 
{
  char *name;
  char *pattern;
} GrokPattern;

typedef struct _GrokParser
{
  LogParser super;
  GList *instances;
  GList *custom_patterns;
  char *grok_pattern_dir;
  char *key_prefix;
  int key_prefix_len;
  LogTemplate *template;
  gboolean debug;
} GrokParser;

GrokInstance *
grok_instance_new(void)
{
  return g_new0(GrokInstance, 1);
}

void 
grok_parser_add_named_subpattern(LogParser *parser, const char *name, const char *pattern)
{
  GrokParser *self = (GrokParser *)parser;
  GrokPattern *grok_pattern = g_new0(GrokPattern, 1);
  grok_pattern->name = g_strdup(name);
  grok_pattern->pattern = g_strdup(pattern);
  self->custom_patterns = g_list_append(self->custom_patterns, grok_pattern);
};

void
grok_instance_add_tags(GrokInstance *instance, GList *tags)
{
  string_list_free(instance->tags);
  instance->tags = tags;
};

void
grok_parser_turn_on_debug(LogParser *parser)
{
  GrokParser *self = (GrokParser *)parser;
  self->debug = TRUE;
}

static void
grok_instance_pattern_list_foreach(gpointer pattern, gpointer user_data)
{
  GrokPattern *grok_pattern = (GrokPattern *)pattern;
  GrokInstance *self = (GrokInstance *)user_data;
  grok_pattern_add(self->grok, grok_pattern->name, strlen(grok_pattern->name), grok_pattern->pattern, strlen(grok_pattern->pattern));
};

static void 
grok_instance_load_named_subpatterns(GrokInstance *self, GrokParser *parser)
{
  g_list_foreach(parser->custom_patterns, grok_instance_pattern_list_foreach, self);
};

void 
grok_instance_set_pattern(GrokInstance *self, char *pattern)
{
  self->grok_pattern = g_strdup(pattern);
};

static void 
grok_instance_free(gpointer obj)
{
  GrokInstance *self = (GrokInstance *) obj;
  string_list_free(self->tags);
  g_free(self->grok_pattern);
  if (self->grok)
    {
      grok_free(self->grok);
      g_free(self->grok);
    }
};

static void
grok_patterns_import_from_directory(GrokInstance *self, GrokParser *parser)
{
  const gchar *fname;

  if (parser->grok_pattern_dir == NULL)
    return;

  GDir *dir = g_dir_open(parser->grok_pattern_dir, 0, NULL);
  if (!dir)
    {
       msg_error("Could not open pattern directory", evt_tag_str("pattern_dir", parser->grok_pattern_dir), NULL);
       return;
    }
 
  while ((fname = g_dir_read_name(dir)))
    {
       gchar *full_name = g_build_filename(parser->grok_pattern_dir, fname, NULL);
       grok_patterns_import_from_file(self->grok, full_name);
       g_free(full_name);
    }
  g_dir_close(dir);
}

static gboolean 
grok_instance_init(GrokInstance *self, GrokParser *parser)
{
  self->grok = g_new0(grok_t, 1);

  grok_init(self->grok);

  if (parser->debug)
     self->grok->logmask = (~0);

  grok_patterns_import_from_directory(self, parser);
  grok_instance_load_named_subpatterns(self, parser);

  if (grok_compile(self->grok, self->grok_pattern) != GROK_OK)
    {
      msg_error("Grok pattern compilation failed", evt_tag_str("error", self->grok->errstr), evt_tag_str("pattern", self->grok_pattern),NULL);
      return FALSE;
    }

  self->key_prefix = parser->key_prefix;
  self->key_prefix_len = parser->key_prefix_len;

  return TRUE;
};

static void
_split_and_add_key_prefix(GrokInstance *self, char *key_buffer, char *key, int key_len)
{
  char *key_start = strchr(key, ':');

  if (key_start == NULL)
    key_start = key;
  else
   {
     key_start = key_start + 1;
     key_len = key_len - (key_start - key);
   }

  if (key_len > (KEY_BUFFER_LENGTH - 1) - self->key_prefix_len) 
    key_len = (KEY_BUFFER_LENGTH - 1) - self->key_prefix_len;

  if (self->key_prefix_len > 0)
    strncpy(key_buffer, self->key_prefix, self->key_prefix_len);

  strncpy(key_buffer + self->key_prefix_len, key_start, key_len);
  key_buffer[key_len + self->key_prefix_len] = '\0'; 
}

static void 
grok_instance_add_matched_values_to_msg(GrokInstance *self, grok_match_t *match, LogMessage *msg)
{
  char *key, *value;
  int key_len, value_len;
  char key_buffer[KEY_BUFFER_LENGTH];

  grok_match_walk_init(match);
  while (grok_match_walk_next(match, &key, &key_len, (const char**) &value, &value_len) == 0)
    {
      _split_and_add_key_prefix(self, key_buffer, key, key_len);

      NVHandle handle = log_msg_get_value_handle(key_buffer);
      log_msg_set_value(msg, handle, value, value_len);
    }
  grok_match_walk_end(match);
}

static void 
_add_tag_to_msg(gpointer data, gpointer userdata)
{
  char *tag = (char *) data;
  LogMessage *msg = (LogMessage *) userdata;
  
  log_msg_set_tag_by_name(msg, tag);
};

static void 
grok_instance_add_tags_to_msg(GrokInstance *self, LogMessage *msg)
{
  g_list_foreach(self->tags, _add_tag_to_msg, msg);
};

static gboolean 
grok_instance_match(GrokInstance *self, char* text, LogMessage *msg)
{
  grok_match_t match;

  int grok_res = grok_exec(self->grok, text, &match);
  if (grok_res == GROK_OK)
    {
      msg_debug("Grok pattern matched!", NULL);
      grok_instance_add_matched_values_to_msg(self, &match, msg);
      grok_instance_add_tags_to_msg(self, msg);
      return TRUE;
    }
  else if (grok_res == GROK_ERROR_NOMATCH)
    {
      msg_debug("Grok pattern not matched!", NULL);
    }
  else if (grok_res == GROK_ERROR_PCRE_ERROR)
    {
      msg_debug("Pcre error happened during grok matching!", NULL);
    }
  return FALSE;
};

static gboolean 
grok_parser_init(LogPipe *parser)
{
  GrokParser *self = (GrokParser *)parser;
  GlobalConfig *cfg = log_pipe_get_config(&self->super.super); 

  if (self->key_prefix != NULL)
    self->key_prefix_len = strlen(self->key_prefix);

  if (!self->template)
  {
    self->template = log_template_new(cfg, "default_grok_template");
    log_template_compile(self->template, "$MESSAGE", NULL);
  }

  g_list_foreach(self->instances, (GFunc) grok_instance_init, self);
  return TRUE;
};

void
grok_parser_set_pattern_directory(LogParser *s, gchar *pattern_directory)
{
  GrokParser *self = (GrokParser *)s;

  if (self->grok_pattern_dir)
    g_free(self->grok_pattern_dir);

  self->grok_pattern_dir = g_strdup(pattern_directory);
}

void
grok_parser_add_pattern_instance(LogParser *s, GrokInstance *instance)
{
  GrokParser *self = (GrokParser *)s;

  self->instances = g_list_append(self->instances, instance);
};

static gboolean 
grok_parser_process(LogParser *s, LogMessage **pmsg, const LogPathOptions *path_options, const char *input, gsize input_len)
{
  LogMessage *msg = *pmsg;
  GString *str;
  GList *instance;
  LogTemplateOptions template_options;
 
  GrokParser *self = (GrokParser *)s;
  str = g_string_new("");
  log_template_options_defaults(&template_options);

  log_template_format(self->template, msg, &template_options, 0, 0, NULL, str);

  instance = self->instances;
  while (instance && !(grok_instance_match(instance->data, str->str, msg)))
    { 
      instance = instance->next;
    }

  g_string_free(str, TRUE);
  return TRUE;
};

static GList* _clone_list(GList* list, gpointer(*item_cloner)(gpointer));

static gpointer
grok_custom_pattern_clone(gpointer obj)
{
  GrokPattern *pattern = (GrokPattern *) obj;
  GrokPattern *cloned_pattern = g_new0(GrokPattern, 1);
  cloned_pattern->name = g_strdup(pattern->name);
  cloned_pattern->pattern = g_strdup(pattern->pattern);
 
  return (gpointer) cloned_pattern;
};

static gpointer
grok_instance_clone(gpointer obj)
{
  GrokInstance *instance = (GrokInstance *) obj;
  GrokInstance *cloned = g_new0(GrokInstance, 1);
  
  cloned->grok_pattern = g_strdup(instance->grok_pattern);
  cloned->tags = _clone_list(instance->tags, g_strdup);
  return (gpointer) cloned;
};

static GList*
_clone_list(GList* list, gpointer(*item_cloner)(gpointer))
{
  GList *result = NULL;
  GList *p;

  for (p = list; p; p = p->next)
    {
      result = g_list_append(result, item_cloner(p->data));
    }
 
  return result; 
}

static GList *
grok_parser_clone_instances(GrokParser *self)
{
  return _clone_list(self->instances, grok_instance_clone);
};

static GList *
grok_parser_clone_custom_patterns(GrokParser *self)
{
  return _clone_list(self->custom_patterns, grok_custom_pattern_clone);
};


static LogPipe *
grok_parser_clone(LogPipe *s)
{
  GrokParser *self = (GrokParser *)s;

  GrokParser *cloned = (GrokParser *) grok_parser_new(log_pipe_get_config(&self->super.super));
  cloned->instances = grok_parser_clone_instances(self);
  cloned->custom_patterns = grok_parser_clone_custom_patterns(self);
  
  cloned->template = log_template_ref(self->template);

  cloned->key_prefix = g_strndup(self->key_prefix, self->key_prefix_len);
  cloned->key_prefix_len = self->key_prefix_len;
  
  cloned->grok_pattern_dir = g_strdup(self->grok_pattern_dir);
  return &cloned->super.super;
};

static void 
grok_pattern_free(gpointer obj)
{
  GrokPattern *self = (GrokPattern *) obj;
  g_free(self->name);
  g_free(self->pattern);
  g_free(self);
};

static void 
grok_parser_free(LogPipe *s)
{
  GrokParser *self = (GrokParser *)s;

  g_list_free_full(self->instances, grok_instance_free);
  g_list_free_full(self->custom_patterns, grok_pattern_free);

  g_free(self->key_prefix);

  log_template_unref(self->template);

  g_free(self->grok_pattern_dir);
  log_parser_free_method(s);
};

LogParser *grok_parser_new(GlobalConfig *cfg)
{
  GrokParser *self = g_new0(GrokParser, 1);
  log_parser_init_instance(&self->super, cfg);
  self->super.super.init = grok_parser_init;
  self->super.process = grok_parser_process;
  self->super.super.clone = grok_parser_clone;
  self->super.super.free_fn = grok_parser_free;
  return &self->super;
};


