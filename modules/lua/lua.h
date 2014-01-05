#ifndef _AFLUA_DEST_H
#define _AFLUA_DEST_H

#include "driver.h"
#include "logwriter.h"
#include <lua.h>

typedef struct _LuaDestDriver
{
  LogDestDriver super;
  lua_State* state;
  gchar* template_string;
  gchar* filename;
  gchar* init_func_name;
  gchar* queue_func_name;
  LogTemplate* template;
  LogTemplateOptions template_options;
} LuaDestDriver;

LogDriver* lua_dd_new();
void lua_dd_set_init_func(LogDriver* d, gchar* init_func_name);
void lua_dd_set_queue_func(LogDriver* d, gchar* queue_func_name);
LogTemplateOptions* lua_dd_get_template_options(LogDriver *d);
#endif
