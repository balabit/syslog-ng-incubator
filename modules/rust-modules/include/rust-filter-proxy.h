#ifndef RUST_FILTER_PROXY_H_INCLUDED
#define RUST_FILTER_PROXY_H_INCLUDED

#include "syslog-ng.h"

struct RustFilter;

void
rust_filter_proxy_free(struct RustFilter* this);

void
rust_filter_proxy_set_option(struct RustFilter* this, const gchar* key, const gchar* value);

gboolean
rust_filter_proxy_eval(struct RustFilter* this, LogMessage* msg);

void
rust_filter_proxy_init(struct RustFilter* s, const GlobalConfig *cfg);

struct RustFilter*
rust_filter_proxy_new(const gchar* filter_name);


#endif
