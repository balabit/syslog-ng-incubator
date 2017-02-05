#include "logmsg.h"
#include "misc.h"
#include "filter.h"
#include "rust-filter-proxy.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct _FilterRust {
  FilterExprNode super;
  struct RustFilterProxy* rust_object;
} FilterRust;

static gboolean
rust_filter_eval(FilterExprNode *s, LogMessage **msgs, gint num_msg)
{
    FilterRust* self = (FilterRust*) s;

    g_assert_cmpint(num_msg, == , 1);

    return rust_filter_proxy_eval(self->rust_object, msgs[0]) ^ s->comp;
}

static void
rust_filter_free(FilterExprNode *s)
{
  FilterRust *self = (FilterRust *)s;

  rust_filter_proxy_free(self->rust_object);
}

void
rust_filter_set_option(FilterExprNode *s, gchar* key, gchar* value)
{    
  FilterRust *self = (FilterRust *)s;

  rust_filter_proxy_set_option(self->rust_object, key, value);
}

void
rust_filter_init(FilterExprNode *s, GlobalConfig *cfg)
{
  FilterRust *self = (FilterRust *)s;

  rust_filter_proxy_init(self->rust_object, cfg);
}

FilterExprNode*
rust_filter_new(const gchar *name)
{
  FilterRust *self = (FilterRust*) g_new0(FilterRust, 1);

  filter_expr_node_init_instance(&self->super);    

  self->rust_object = rust_filter_proxy_new(name);

  if (!self->rust_object)
    return NULL;

  self->super.eval = rust_filter_eval;
  self->super.free_fn = rust_filter_free;
  self->super.init = rust_filter_init;

  return &self->super;
}

