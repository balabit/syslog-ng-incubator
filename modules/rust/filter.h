#ifndef RUST_FILTER_H_INCLUDED
#define RUST_FILTER_H_INCLUDED

#include "filter/filter-expr.h"

FilterExprNode *rust_filter_new(const gchar* name);
void rust_filter_set_option(FilterExprNode *s, gchar* key, gchar* value);

#endif
