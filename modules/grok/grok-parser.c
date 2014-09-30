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
  GlobalConfig *config;
  gboolean debug;
} GrokParser;

GrokInstance *grok_instance_new()
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
grok_instance_add_tag(GrokInstance *instance, const char *tag)
{
  instance->tags = g_list_append(instance->tags, tag);
};

void
grok_parser_turn_on_debug(LogParser *parser)
{
  GrokParser *self = (GrokParser *)parser;
  self->debug = TRUE;
}

void
grok_instance_pattern_list_foreach(gpointer pattern, gpointer user_data)
{
  GrokPattern *grok_pattern = (GrokPattern *)pattern;
  GrokInstance *self = (GrokInstance *)user_data;
  grok_pattern_add(self->grok, grok_pattern->name, strlen(grok_pattern->name), grok_pattern->pattern, strlen(grok_pattern->pattern));
};

void 
grok_instance_load_named_subpatterns(GrokInstance *self, GrokParser *parser)
{
  g_list_foreach(parser->custom_patterns, grok_instance_pattern_list_foreach, self);
};

void grok_instance_set_pattern(GrokInstance *self, char *pattern)
{
  self->grok_pattern = g_strdup(pattern);
};

void grok_instance_free(GrokInstance *self)
{
  g_list_free_full(self->tags, g_free);
  g_free(self->grok_pattern);
  grok_free(self->grok);
  g_free(self->grok);
};

void
grok_patterns_import_from_directory(GrokInstance *self, GrokParser *parser)
{
  gchar *fname;

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

gboolean grok_instance_init(GrokInstance *self, GrokParser *parser)
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

void grok_instance_add_matched_values_to_msg(GrokInstance *self, grok_match_t *match, LogMessage *msg)
{
  char *key, *value;
  int key_len, value_len;
  char key_buffer[1024];

  grok_match_walk_init(match);
  while (grok_match_walk_next(match, &key, &key_len, &value, &value_len) == 0)
    {
      fprintf(stderr, "WALK: '%.*s' : '%.*s'\n", key_len, key, value_len, value);
      char *key_start = strchr(key, ':');
      if (key_start == NULL)
         key_start = key;
      else
         {
           key_start = key_start + 1;
           key_len = key_len - (key_start - key);
         }
      if (key_len > 1023 - self->key_prefix_len) 
        key_len = 1023 - self->key_prefix_len;
      if (self->key_prefix_len > 0)
         strncpy(key_buffer, self->key_prefix, self->key_prefix_len);
      strncpy(key_buffer + self->key_prefix_len, key_start, key_len);
      fprintf(stderr, "KEY: %s", key_buffer);
      key_buffer[key_len+self->key_prefix_len] = '\0';
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

gboolean 
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

gboolean grok_parser_init(LogParser *parser, GlobalConfig *cfg)
{
  GrokParser *self = (GrokParser *)parser;

  if (self->key_prefix != NULL)
    self->key_prefix_len = strlen(self->key_prefix);

  if (!self->template)
  {
    self->template = log_template_new(cfg, "default_grok_template");
    log_template_compile(self->template, "$MESSAGE", NULL);
  }

  g_list_foreach(self->instances, grok_instance_init, self);
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

gboolean grok_parser_process(LogParser *s, LogMessage **pmsg, const LogPathOptions *path_options, const char *input, gsize input_len)
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

void grok_pattern_free(GrokPattern *self)
{
  g_free(self->name);
  g_free(self->pattern);
  g_free(self);
};

void grok_parser_free(LogParser *s)
{
  GrokParser *self = (GrokParser *)s;

  g_list_free_full(self->instances, grok_instance_free);
  g_list_free_full(self->custom_patterns, grok_pattern_free);

  g_free(self->grok_pattern_dir);
  log_parser_free_method(s);
};

LogParser *grok_parser_new()
{
  GrokParser *self = g_new0(GrokParser, 1);
  log_parser_init_instance(&self->super);
  self->super.super.init = grok_parser_init;
  self->super.process = grok_parser_process;
  self->super.super.free_fn = grok_parser_free;
  return &self->super;
};


