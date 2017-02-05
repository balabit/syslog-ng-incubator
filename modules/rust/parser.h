#ifndef RUST_PARSER_H_INCLUDED
#define RUST_PARSER_H_INCLUDED

#include "parser/parser-expr.h"

LogParser *rust_parser_new(const gchar* name, GlobalConfig *cfg);
void rust_parser_set_option(LogParser *s, gchar* key, gchar* value);

#endif
