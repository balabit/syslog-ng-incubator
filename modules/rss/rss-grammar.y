/*
 * Copyright (c) 2013 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 2013 Tusa Viktor
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

%code requires {

#include "rss-parser.h"

}

%code {

#include "cfg-parser.h"
#include "rss-grammar.h"
#include "plugin.h"

extern LogDriver *last_driver;

}

%name-prefix "rss_"
%lex-param {CfgLexer *lexer}
%parse-param {CfgLexer *lexer}
%parse-param {LogDriver **instance}
%parse-param {gpointer arg}



%require "2.4.1"
%locations
%define api.pure
%pure-parser
%error-verbose

%code {

# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
  do {                                                                  \
    if (N)                                                              \
      {                                                                 \
        (Current).level = YYRHSLOC(Rhs, 1).level;                       \
        (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;          \
        (Current).first_column = YYRHSLOC (Rhs, 1).first_column;        \
        (Current).last_line    = YYRHSLOC (Rhs, N).last_line;           \
        (Current).last_column  = YYRHSLOC (Rhs, N).last_column;         \
      }                                                                 \
    else                                                                \
      {                                                                 \
        (Current).level = YYRHSLOC(Rhs, 0).level;                       \
        (Current).first_line   = (Current).last_line   =                \
          YYRHSLOC (Rhs, 0).last_line;                                  \
        (Current).first_column = (Current).last_column =                \
          YYRHSLOC (Rhs, 0).last_column;                                \
      }                                                                 \
  } while (0)

#define CHECK_ERROR_WITHOUT_MESSAGE(val, token) do {                    \
    if (!(val))                                                         \
      {                                                                 \
        YYERROR;                                                        \
      }                                                                 \
  } while (0)

#define CHECK_ERROR(val, token, errorfmt, ...) do {                     \
    if (!(val))                                                         \
      {                                                                 \
        if (errorfmt)                                                   \
          {                                                             \
            gchar __buf[256];                                           \
            g_snprintf(__buf, sizeof(__buf), errorfmt, ## __VA_ARGS__); \
            yyerror(& (token), lexer, NULL, NULL, __buf);               \
          }                                                             \
        YYERROR;                                                        \
      }                                                                 \
  } while (0)

#define CHECK_ERROR_GERROR(val, token, error, errorfmt, ...) do {       \
    if (!(val))                                                         \
      {                                                                 \
        if (errorfmt)                                                   \
          {                                                             \
            gchar __buf[256];                                           \
            g_snprintf(__buf, sizeof(__buf), errorfmt ", error=%s", ## __VA_ARGS__, error->message); \
            yyerror(& (token), lexer, NULL, NULL, __buf);               \
          }                                                             \
        g_clear_error(&error);						\
        YYERROR;                                                        \
      }                                                                 \
  } while (0)

#define YYMAXDEPTH 20000


}

/* plugin types, must be equal to the numerical values of the plugin type in plugin.h */

%token LL_CONTEXT_ROOT                1
%token LL_CONTEXT_DESTINATION         2
%token LL_CONTEXT_SOURCE              3
%token LL_CONTEXT_PARSER              4
%token LL_CONTEXT_REWRITE             5
%token LL_CONTEXT_FILTER              6
%token LL_CONTEXT_LOG                 7
%token LL_CONTEXT_BLOCK_DEF           8
%token LL_CONTEXT_BLOCK_REF           9
%token LL_CONTEXT_BLOCK_CONTENT       10
%token LL_CONTEXT_BLOCK_ARG           11
%token LL_CONTEXT_PRAGMA              12
%token LL_CONTEXT_FORMAT              13
%token LL_CONTEXT_TEMPLATE_FUNC       14
%token LL_CONTEXT_INNER_DEST          15
%token LL_CONTEXT_INNER_SRC           16
%token LL_CONTEXT_CLIENT_PROTO        17
%token LL_CONTEXT_SERVER_PROTO        18

/* statements */
%token KW_SOURCE                      10000
%token KW_FILTER                      10001
%token KW_PARSER                      10002
%token KW_DESTINATION                 10003
%token KW_LOG                         10004
%token KW_OPTIONS                     10005
%token KW_INCLUDE                     10006
%token KW_BLOCK                       10007
%token KW_JUNCTION                    10008
%token KW_CHANNEL                     10009

/* source & destination items */
%token KW_INTERNAL                    10010
%token KW_FILE                        10011

%token KW_SQL                         10030
%token KW_TYPE                        10031
%token KW_COLUMNS                     10032
%token KW_INDEXES                     10033
%token KW_VALUES                      10034
%token KW_PASSWORD                    10035
%token KW_DATABASE                    10036
%token KW_USERNAME                    10037
%token KW_TABLE                       10038
%token KW_ENCODING                    10039
%token KW_SESSION_STATEMENTS          10040

%token KW_DELIMITERS                  10050
%token KW_QUOTES                      10051
%token KW_QUOTE_PAIRS                 10052
%token KW_NULL                        10053

%token KW_SYSLOG                      10060

/* option items */
%token KW_MARK_FREQ                   10071
%token KW_STATS_FREQ                  10072
%token KW_STATS_LEVEL                 10073
%token KW_STATS_LIFETIME	      10074
%token KW_FLUSH_LINES                 10075
%token KW_SUPPRESS                    10076
%token KW_FLUSH_TIMEOUT               10077
%token KW_LOG_MSG_SIZE                10078
%token KW_FILE_TEMPLATE               10079
%token KW_PROTO_TEMPLATE              10080
%token KW_MARK_MODE                   10081

%token KW_CHAIN_HOSTNAMES             10090
%token KW_NORMALIZE_HOSTNAMES         10091
%token KW_KEEP_HOSTNAME               10092
%token KW_CHECK_HOSTNAME              10093
%token KW_BAD_HOSTNAME                10094

%token KW_KEEP_TIMESTAMP              10100

%token KW_USE_DNS                     10110
%token KW_USE_FQDN                    10111
%token KW_CUSTOM_DOMAIN	              10112

%token KW_DNS_CACHE                   10120
%token KW_DNS_CACHE_SIZE              10121

%token KW_DNS_CACHE_EXPIRE            10130
%token KW_DNS_CACHE_EXPIRE_FAILED     10131
%token KW_DNS_CACHE_HOSTS             10132

%token KW_PERSIST_ONLY                10140
%token KW_USE_RCPTID                  10141

%token KW_TZ_CONVERT                  10150
%token KW_TS_FORMAT                   10151
%token KW_FRAC_DIGITS                 10152

%token KW_LOG_FIFO_SIZE               10160
%token KW_LOG_FETCH_LIMIT             10162
%token KW_LOG_IW_SIZE                 10163
%token KW_LOG_PREFIX                  10164
%token KW_PROGRAM_OVERRIDE            10165
%token KW_HOST_OVERRIDE               10166

%token KW_THROTTLE                    10170
%token KW_THREADED                    10171

/* log statement options */
%token KW_FLAGS                       10190

/* reader options */
%token KW_PAD_SIZE                    10200
%token KW_TIME_ZONE                   10201
%token KW_RECV_TIME_ZONE              10202
%token KW_SEND_TIME_ZONE              10203
%token KW_LOCAL_TIME_ZONE             10204
%token KW_FORMAT                      10205

/* timers */
%token KW_TIME_REOPEN                 10210
%token KW_TIME_REAP                   10211
%token KW_TIME_SLEEP                  10212

/* destination options */
%token KW_TMPL_ESCAPE                 10220

/* driver specific options */
%token KW_OPTIONAL                    10230

/* file related options */
%token KW_CREATE_DIRS                 10240

%token KW_OWNER                       10250
%token KW_GROUP                       10251
%token KW_PERM                        10252

%token KW_DIR_OWNER                   10260
%token KW_DIR_GROUP                   10261
%token KW_DIR_PERM                    10262

%token KW_TEMPLATE                    10270
%token KW_TEMPLATE_ESCAPE             10271

%token KW_DEFAULT_FACILITY            10300
%token KW_DEFAULT_LEVEL               10301

%token KW_PORT                        10323
/* misc options */

%token KW_USE_TIME_RECVD              10340

/* filter items*/
%token KW_FACILITY                    10350
%token KW_LEVEL                       10351
%token KW_HOST                        10352
%token KW_MATCH                       10353
%token KW_MESSAGE                     10354
%token KW_NETMASK                     10355
%token KW_TAGS                        10356

/* parser items */

%token KW_VALUE                       10361

/* rewrite items */

%token KW_REWRITE                     10370
%token KW_SET                         10371
%token KW_SUBST                       10372

/* yes/no switches */

%token KW_YES                         10380
%token KW_NO                          10381

%token KW_IFDEF                       10410
%token KW_ENDIF                       10411

%token LL_DOTDOT                      10420

%token <cptr> LL_IDENTIFIER           10421
%token <num>  LL_NUMBER               10422
%token <fnum> LL_FLOAT                10423
%token <cptr> LL_STRING               10424
%token <token> LL_TOKEN               10425
%token <cptr> LL_BLOCK                10426
%token LL_PRAGMA                      10427
%token LL_EOL                         10428
%token LL_ERROR                       10429

/* value pairs */
%token KW_VALUE_PAIRS                 10500
%token KW_SELECT                      10501
%token KW_EXCLUDE                     10502
%token KW_PAIR                        10503
%token KW_KEY                         10504
%token KW_SCOPE                       10505
%token KW_SHIFT                       10506
%token KW_REKEY                       10507
%token KW_ADD_PREFIX                  10508
%token KW_REPLACE_PREFIX              10509

%token KW_ON_ERROR                    10510


%type   <ptr> source_content
%type	<ptr> source_items
%type	<ptr> source_item
%type   <ptr> source_afinter
%type   <ptr> source_plugin
%type   <ptr> source_afinter_params

%type   <ptr> dest_content
%type	<ptr> dest_items
%type	<ptr> dest_item
%type   <ptr> dest_plugin

%type   <ptr> template_content

%type   <ptr> filter_content

%type   <ptr> parser_content

%type   <ptr> rewrite_content

%type	<ptr> log_items
%type	<ptr> log_item

%type   <ptr> log_last_junction
%type   <ptr> log_junction
%type   <ptr> log_content
%type   <ptr> log_forks
%type   <ptr> log_fork

%type	<num> log_flags
%type   <num> log_flags_items

%type   <ptr> value_pair_option

%type	<num> yesno
%type   <num> dnsmode
%type	<num> dest_writer_options_flags

%type	<cptr> string
%type	<cptr> string_or_number
%type   <ptr> string_list
%type   <ptr> string_list_build
%type   <num> facility_string
%type   <num> level_string


%token KW_RSS
%token KW_RSS_PORT
%token KW_RSS_TITLE
%token KW_RSS_ENTRY_TITLE
%token KW_RSS_ENTRY_DESCRIPTION

%%

start
        : LL_CONTEXT_DESTINATION KW_RSS
          {
            last_driver = *instance = rss_dd_new(configuration);
          }
          '(' rss_options ')'         { YYACCEPT; }
        ;

rss_options
        : rss_option rss_options
        |
        ;

rss_option
        : KW_RSS_PORT '(' LL_NUMBER ')'    { rss_dd_set_port(last_driver,$3); };
        | KW_RSS_TITLE '(' string ')'    { rss_dd_set_title(last_driver, $3); free($3); };
        | KW_RSS_ENTRY_TITLE '(' string ')' { rss_dd_set_entry_title(last_driver, $3); free($3); };
        | KW_RSS_ENTRY_DESCRIPTION '(' string ')' { rss_dd_set_entry_description(last_driver, $3); free($3); };
        | dest_driver_option
        ;



source_content
        :
          { cfg_lexer_push_context(lexer, LL_CONTEXT_SOURCE, NULL, "source"); }
          source_items
          { cfg_lexer_pop_context(lexer); }
          {
            $$ = log_expr_node_new_junction($2, &@$);
          }
        ;

source_items
        : source_item semicolons source_items	{ $$ = log_expr_node_append_tail(log_expr_node_new_pipe($1, &@1), $3); }
        | log_fork semicolons source_items      { $$ = log_expr_node_append_tail($1,  $3); }
	|					{ $$ = NULL; }
	;

source_item
	: source_afinter			{ $$ = $1; }
        | source_plugin                         { $$ = $1; }
	;

source_plugin
        : LL_IDENTIFIER
          {
            Plugin *p;
            gint context = LL_CONTEXT_SOURCE;

            p = plugin_find(configuration, context, $1);
            CHECK_ERROR(p, @1, "%s plugin %s not found", cfg_lexer_lookup_context_name_by_type(context), $1);

            last_driver = (LogDriver *) plugin_parse_config(p, configuration, &@1, NULL);
            free($1);
            if (!last_driver)
              {
                YYERROR;
              }
            $$ = last_driver;
          }
        ;

source_afinter
	: KW_INTERNAL '(' source_afinter_params ')'			{ $$ = $3; }
	;

source_afinter_params
        : {
            last_driver = afinter_sd_new(configuration);
            last_source_options = &((AFInterSourceDriver *) last_driver)->source_options;
          }
          source_afinter_options { $$ = last_driver; }
        ;

source_afinter_options
        : source_afinter_option source_afinter_options
        |
        ;

source_afinter_option
        : source_option
        ;


filter_content
        : {
            FilterExprNode *last_filter_expr = NULL;

	    CHECK_ERROR_WITHOUT_MESSAGE(cfg_parser_parse(&filter_expr_parser, lexer, (gpointer *) &last_filter_expr, NULL), @$);

            $$ = log_expr_node_new_pipe(log_filter_pipe_new(last_filter_expr, configuration), &@$);
	  }
	;

parser_content
        :
          {
            LogExprNode *last_parser_expr = NULL;

            CHECK_ERROR_WITHOUT_MESSAGE(cfg_parser_parse(&parser_expr_parser, lexer, (gpointer *) &last_parser_expr, NULL), @$);
            $$ = last_parser_expr;
          }
        ;

rewrite_content
        :
          {
            LogExprNode *last_rewrite_expr = NULL;

            CHECK_ERROR_WITHOUT_MESSAGE(cfg_parser_parse(&rewrite_expr_parser, lexer, (gpointer *) &last_rewrite_expr, NULL), @$);
            $$ = last_rewrite_expr;
          }
        ;

dest_content
         : { cfg_lexer_push_context(lexer, LL_CONTEXT_DESTINATION, NULL, "destination"); }
            dest_items
           { cfg_lexer_pop_context(lexer); }
           {
             $$ = log_expr_node_new_junction($2, &@$);
           }
         ;


dest_items
        /* all destination drivers are added as an independent branch in a junction*/
        : dest_item semicolons dest_items	{ $$ = log_expr_node_append_tail(log_expr_node_new_pipe($1, &@1), $3); }
        | log_fork semicolons dest_items        { $$ = log_expr_node_append_tail($1,  $3); }
	|					{ $$ = NULL; }
	;

dest_item
        : dest_plugin                           { $$ = $1; }
	;

dest_plugin
        : LL_IDENTIFIER
          {
            Plugin *p;
            gint context = LL_CONTEXT_DESTINATION;

            p = plugin_find(configuration, context, $1);
            CHECK_ERROR(p, @1, "%s plugin %s not found", cfg_lexer_lookup_context_name_by_type(context), $1);

            last_driver = (LogDriver *) plugin_parse_config(p, configuration, &@1, NULL);
            free($1);
            if (!last_driver)
              {
                YYERROR;
              }
            $$ = last_driver;
          }
        ;

log_items
	: log_item semicolons log_items		{ log_expr_node_append_tail($1, $3); $$ = $1; }
	|					{ $$ = NULL; }
	;

log_item
        : KW_SOURCE '(' string ')'		{ $$ = log_expr_node_new_source_reference($3, &@$); free($3); }
        | KW_SOURCE '{' source_content '}'      { $$ = log_expr_node_new_source(NULL, $3, &@$); }
        | KW_FILTER '(' string ')'		{ $$ = log_expr_node_new_filter_reference($3, &@$); free($3); }
        | KW_FILTER '{' filter_content '}'      { $$ = log_expr_node_new_filter(NULL, $3, &@$); }
        | KW_PARSER '(' string ')'              { $$ = log_expr_node_new_parser_reference($3, &@$); free($3); }
        | KW_PARSER '{' parser_content '}'      { $$ = log_expr_node_new_parser(NULL, $3, &@$); }
        | KW_REWRITE '(' string ')'             { $$ = log_expr_node_new_rewrite_reference($3, &@$); free($3); }
        | KW_REWRITE '{' rewrite_content '}'    { $$ = log_expr_node_new_rewrite(NULL, $3, &@$); }
        | KW_DESTINATION '(' string ')'		{ $$ = log_expr_node_new_destination_reference($3, &@$); free($3); }
        | KW_DESTINATION '{' dest_content '}'   { $$ = log_expr_node_new_destination(NULL, $3, &@$); }
        | log_junction                          { $$ = $1; }
	;

log_junction
        : KW_JUNCTION '{' log_forks '}'         { $$ = log_expr_node_new_junction($3, &@$); }
        ;

log_last_junction

        /* this rule matches the last set of embedded log {}
         * statements at the end of the log {} block.
         * It is the final junction and was the only form of creating
         * a processing tree before syslog-ng 3.4.
         *
         * We emulate if the user was writing junction {} explicitly.
         */
        : log_forks                             { $$ = $1 ? log_expr_node_new_junction($1, &@1) :  NULL; }
        ;


log_forks
        : log_fork semicolons log_forks		{ log_expr_node_append_tail($1, $3); $$ = $1; }
        |                                       { $$ = NULL; }
        ;

log_fork
        : KW_LOG '{' log_content '}'            { $$ = $3; }
        | KW_CHANNEL '{' log_content '}'        { $$ = $3; }
        ;

log_content
        : log_items log_last_junction log_flags                { $$ = log_expr_node_new_log(log_expr_node_append_tail($1, $2), $3, &@$); }
        ;

log_flags
	: KW_FLAGS '(' log_flags_items ')' semicolons	{ $$ = $3; }
	|					{ $$ = 0; }
	;

log_flags_items
	: string log_flags_items		{ $$ = log_expr_node_lookup_flag($1) | $2; free($1); }
	|					{ $$ = 0; }
	;


template_content_inner
        : string
        {
          GError *error = NULL;

          CHECK_ERROR(log_template_compile(last_template, $1, &error), @1, "Error compiling template (%s)", error->message);
          free($1);
        }
        | LL_IDENTIFIER '(' string ')'
        {
          GError *error = NULL;

          CHECK_ERROR(log_template_compile(last_template, $3, &error), @3, "Error compiling template (%s)", error->message);
          free($3);

          CHECK_ERROR(log_template_set_type_hint(last_template, $1, &error), @1, "Error setting the template type-hint (%s)", error->message);
          free($1);
        }
        ;

template_content
        : { last_template = log_template_new(configuration, NULL); } template_content_inner	{ $$ = last_template; }
        ;



string
	: LL_IDENTIFIER
	| LL_STRING
	;

yesno
	: KW_YES				{ $$ = 1; }
	| KW_NO					{ $$ = 0; }
	| LL_NUMBER				{ $$ = $1; }
	;

dnsmode
	: yesno					{ $$ = $1; }
	| KW_PERSIST_ONLY                       { $$ = 2; }
	;

string_or_number
        : string                                { $$ = $1; }
        | LL_NUMBER                             { $$ = strdup(lexer->token_text->str); }
        | LL_FLOAT                              { $$ = strdup(lexer->token_text->str); }
        ;

string_list
        : string_list_build                     { $$ = g_list_reverse($1); }
        ;

string_list_build
        : string string_list_build		{ $$ = g_list_append($2, g_strdup($1)); free($1); }
        |					{ $$ = NULL; }
        ;

semicolons
        : ';'
        | ';' semicolons
        ;

level_string
        : string
	  {
	    /* return the numeric value of the "level" */
	    int n = syslog_name_lookup_level_by_name($1);
	    CHECK_ERROR((n != -1), @1, "Unknown priority level\"%s\"", $1);
	    free($1);
            $$ = n;
	  }
        ;

facility_string
        : string
          {
            /* return the numeric value of facility */
	    int n = syslog_name_lookup_facility_by_name($1);
	    CHECK_ERROR((n != -1), @1, "Unknown facility \"%s\"", $1);
	    free($1);
	    $$ = n;
	  }
        | KW_SYSLOG 				{ $$ = LOG_SYSLOG; }
        ;

parser_opt
        : KW_TEMPLATE '(' string ')'            {
                                                  LogTemplate *template;
                                                  GError *error = NULL;

                                                  template = cfg_tree_check_inline_template(&configuration->tree, $3, &error);
                                                  CHECK_ERROR_GERROR(template != NULL, @3, error, "Error compiling template");
                                                  log_parser_set_template(last_parser, template);
                                                  free($3);
                                                }
        ;

parser_column_opt
        : parser_opt
        | KW_COLUMNS '(' string_list ')'        { log_column_parser_set_columns((LogColumnParser *) last_parser, $3); }
        ;

/* LogSource related options */
source_option
        /* NOTE: plugins need to set "last_source_options" in order to incorporate this rule in their grammar */
	: KW_LOG_IW_SIZE '(' LL_NUMBER ')'	{ last_source_options->init_window_size = $3; }
	| KW_CHAIN_HOSTNAMES '(' yesno ')'	{ last_source_options->chain_hostnames = $3; }
	| KW_KEEP_HOSTNAME '(' yesno ')'	{ last_source_options->keep_hostname = $3; }
	| KW_PROGRAM_OVERRIDE '(' string ')'	{ last_source_options->program_override = g_strdup($3); free($3); }
	| KW_HOST_OVERRIDE '(' string ')'	{ last_source_options->host_override = g_strdup($3); free($3); }
	| KW_LOG_PREFIX '(' string ')'	        { gchar *p = strrchr($3, ':'); if (p) *p = 0; last_source_options->program_override = g_strdup($3); free($3); }
	| KW_KEEP_TIMESTAMP '(' yesno ')'	{ last_source_options->keep_timestamp = $3; }
        | KW_TAGS '(' string_list ')'		{ log_source_options_set_tags(last_source_options, $3); }
        | { last_host_resolve_options = &last_source_options->host_resolve_options; } host_resolve_option
        ;

source_proto_option
        : KW_ENCODING '(' string ')'		{ last_proto_server_options->encoding = g_strdup($3); free($3); }
	| KW_LOG_MSG_SIZE '(' LL_NUMBER ')'	{ last_proto_server_options->max_msg_size = $3; }
        ;

source_reader_options
	: source_reader_option source_reader_options
	;

host_resolve_option
        : KW_USE_FQDN '(' yesno ')'             { last_host_resolve_options->use_fqdn = $3; }
        | KW_USE_DNS '(' dnsmode ')'            { last_host_resolve_options->use_dns = $3; }
	| KW_DNS_CACHE '(' yesno ')' 		{ last_host_resolve_options->use_dns_cache = $3; }
	| KW_NORMALIZE_HOSTNAMES '(' yesno ')'	{ last_host_resolve_options->normalize_hostnames = $3; }
	;

msg_format_option
	: KW_TIME_ZONE '(' string ')'		{ last_msg_format_options->recv_time_zone = g_strdup($3); free($3); }
	| KW_DEFAULT_LEVEL '(' level_string ')'
	  {
	    if (last_msg_format_options->default_pri == 0xFFFF)
	      last_msg_format_options->default_pri = LOG_USER;
	    last_msg_format_options->default_pri = (last_msg_format_options->default_pri & ~7) | $3;
          }
	| KW_DEFAULT_FACILITY '(' facility_string ')'
	  {
	    if (last_msg_format_options->default_pri == 0xFFFF)
	      last_msg_format_options->default_pri = LOG_NOTICE;
	    last_msg_format_options->default_pri = (last_msg_format_options->default_pri & 7) | $3;
          }
        ;


/* LogReader related options, inherits from LogSource */
source_reader_option
        /* NOTE: plugins need to set "last_reader_options" in order to incorporate this rule in their grammar */

	: KW_CHECK_HOSTNAME '(' yesno ')'	{ last_reader_options->check_hostname = $3; }
	| KW_FLAGS '(' source_reader_option_flags ')'
	| KW_LOG_FETCH_LIMIT '(' LL_NUMBER ')'	{ last_reader_options->fetch_limit = $3; }
        | KW_FORMAT '(' string ')'              { last_reader_options->parse_options.format = g_strdup($3); free($3); }
        | { last_source_options = &last_reader_options->super; } source_option
        | { last_proto_server_options = &last_reader_options->proto_options.super; } source_proto_option
        | { last_msg_format_options = &last_reader_options->parse_options; } msg_format_option
	;

source_reader_option_flags
        : string source_reader_option_flags     { CHECK_ERROR(log_reader_options_process_flag(last_reader_options, $1), @1, "Unknown flag %s", $1); free($1); }
        | KW_CHECK_HOSTNAME source_reader_option_flags     { log_reader_options_process_flag(last_reader_options, "check-hostname"); }
	|
	;

dest_driver_option
        /* NOTE: plugins need to set "last_driver" in order to incorporate this rule in their grammar */

	: KW_LOG_FIFO_SIZE '(' LL_NUMBER ')'	{ ((LogDestDriver *) last_driver)->log_fifo_size = $3; }
	| KW_THROTTLE '(' LL_NUMBER ')'         { ((LogDestDriver *) last_driver)->throttle = $3; }
        | LL_IDENTIFIER
          {
            Plugin *p;
            gint context = LL_CONTEXT_INNER_DEST;
            gpointer value;

            p = plugin_find(configuration, context, $1);
            CHECK_ERROR(p, @1, "%s plugin %s not found", cfg_lexer_lookup_context_name_by_type(context), $1);

            value = plugin_parse_config(p, configuration, &@1, last_driver);

            free($1);
            if (!value)
              {
                YYERROR;
              }
            log_driver_add_plugin(last_driver, (LogDriverPlugin *) value);
          }
        ;

dest_writer_options
	: dest_writer_option dest_writer_options
	|
	;

dest_writer_option
        /* NOTE: plugins need to set "last_writer_options" in order to incorporate this rule in their grammar */

	: KW_FLAGS '(' dest_writer_options_flags ')' { last_writer_options->options = $3; }
	| KW_FLUSH_LINES '(' LL_NUMBER ')'		{ last_writer_options->flush_lines = $3; }
	| KW_FLUSH_TIMEOUT '(' LL_NUMBER ')'	{ last_writer_options->flush_timeout = $3; }
        | KW_SUPPRESS '(' LL_NUMBER ')'            { last_writer_options->suppress = $3; }
	| KW_TEMPLATE '(' string ')'       	{
                                                  GError *error = NULL;

                                                  last_writer_options->template = cfg_tree_check_inline_template(&configuration->tree, $3, &error);
                                                  CHECK_ERROR_GERROR(last_writer_options->template != NULL, @3, error, "Error compiling template");
	                                          free($3);
	                                        }
	| KW_TEMPLATE_ESCAPE '(' yesno ')'	{ log_writer_options_set_template_escape(last_writer_options, $3); }
	| KW_PAD_SIZE '(' LL_NUMBER ')'         { last_writer_options->padding = $3; }
	| KW_MARK_FREQ '(' LL_NUMBER ')'        { last_writer_options->mark_freq = $3; }
        | KW_MARK_MODE '(' KW_INTERNAL ')'      { log_writer_options_set_mark_mode(last_writer_options, "internal"); }
	| KW_MARK_MODE '(' string ')'
	  {
	    CHECK_ERROR(cfg_lookup_mark_mode($3) != -1, @3, "illegal mark mode: %s", $3);
            log_writer_options_set_mark_mode(last_writer_options, $3);
            free($3);
          }
        | { last_template_options = &last_writer_options->template_options; } template_option
	;

dest_writer_options_flags
	: string dest_writer_options_flags      { $$ = log_writer_options_lookup_flag($1) | $2; free($1); }
	|					{ $$ = 0; }
	;

file_perm_option
	: KW_OWNER '(' string_or_number ')'	{ file_perm_options_set_file_uid(last_file_perm_options, $3); free($3); }
	| KW_OWNER '(' ')'	                { file_perm_options_set_file_uid(last_file_perm_options, "-2"); }
	| KW_GROUP '(' string_or_number ')'	{ file_perm_options_set_file_gid(last_file_perm_options, $3); free($3); }
	| KW_GROUP '(' ')'	                { file_perm_options_set_file_gid(last_file_perm_options, "-2"); }
	| KW_PERM '(' LL_NUMBER ')'		{ file_perm_options_set_file_perm(last_file_perm_options, $3); }
	| KW_PERM '(' ')'		        { file_perm_options_set_file_perm(last_file_perm_options, -2); }
        ;

file_dir_perm_option
        : file_perm_option
        | KW_DIR_OWNER '(' string_or_number ')'	{ file_perm_options_set_dir_uid(last_file_perm_options, $3); free($3); }
	| KW_DIR_OWNER '(' ')'	                { file_perm_options_set_dir_uid(last_file_perm_options, "-2"); }
	| KW_DIR_GROUP '(' string_or_number ')'	{ file_perm_options_set_dir_gid(last_file_perm_options, $3); free($3); }
	| KW_DIR_GROUP '(' ')'	                { file_perm_options_set_dir_gid(last_file_perm_options, "-2"); }
	| KW_DIR_PERM '(' LL_NUMBER ')'		{ file_perm_options_set_dir_perm(last_file_perm_options, $3); }
	| KW_DIR_PERM '(' ')'		        { file_perm_options_set_dir_perm(last_file_perm_options, -2); }
        ;

template_option
	: KW_TS_FORMAT '(' string ')'		{ last_template_options->ts_format = cfg_ts_format_value($3); free($3); }
	| KW_FRAC_DIGITS '(' LL_NUMBER ')'	{ last_template_options->frac_digits = $3; }
	| KW_TIME_ZONE '(' string ')'		{ last_template_options->time_zone[LTZ_SEND] = g_strdup($3); free($3); }
	| KW_SEND_TIME_ZONE '(' string ')'      { last_template_options->time_zone[LTZ_SEND] = g_strdup($3); free($3); }
	| KW_LOCAL_TIME_ZONE '(' string ')'     { last_template_options->time_zone[LTZ_LOCAL] = g_strdup($3); free($3); }
	| KW_ON_ERROR '(' string ')'
        {
          gint on_error;

          CHECK_ERROR(log_template_on_error_parse($3, &on_error), @3, "Invalid on-error() setting");
          free($3);

          log_template_options_set_on_error(last_template_options, on_error);
        }
	;

matcher_option
        : KW_TYPE '(' string ')'		{ CHECK_ERROR(log_matcher_options_set_type(last_matcher_options, $3), @3, "unknown matcher type"); free($3); }
        | KW_FLAGS '(' matcher_flags ')'
        ;

matcher_flags
        : string matcher_flags			{ CHECK_ERROR(log_matcher_options_process_flag(last_matcher_options, $1), @1, "unknown matcher flag"); free($1); }
        |
        ;

value_pair_option
	: KW_VALUE_PAIRS
          {
            last_value_pairs = value_pairs_new();
          }
          '(' vp_options ')'
          { $$ = last_value_pairs; }
	;

vp_options
	: vp_option vp_options
	|
	;

vp_option
        : KW_PAIR '(' string ':' template_content ')'
        {
          value_pairs_add_pair(last_value_pairs, $3, $5);
          free($3);
        }
        | KW_PAIR '(' string template_content ')'
        {
          value_pairs_add_pair(last_value_pairs, $3, $4);
          free($3);
        }
        | KW_KEY '(' string KW_REKEY '('
        {
          last_vp_transset = value_pairs_transform_set_new($3);
          value_pairs_add_glob_pattern(last_value_pairs, $3, TRUE);
          free($3);
        }
        vp_rekey_options
        ')' { value_pairs_add_transforms(last_value_pairs, last_vp_transset); } ')'
	| KW_KEY '(' string ')'		         { value_pairs_add_glob_pattern(last_value_pairs, $3, TRUE); free($3);  }
        | KW_REKEY '(' string
        {
          last_vp_transset = value_pairs_transform_set_new($3);
          free($3);
        }
        vp_rekey_options ')'                     { value_pairs_add_transforms(last_value_pairs, last_vp_transset); }
	| KW_EXCLUDE '(' string ')'	         { value_pairs_add_glob_pattern(last_value_pairs, $3, FALSE); free($3); }
	| KW_SCOPE '(' vp_scope_list ')'
	;

vp_scope_list
	: string vp_scope_list              { value_pairs_add_scope(last_value_pairs, $1); free($1); }
	|
	;

vp_rekey_options
	: vp_rekey_option vp_rekey_options
        |
	;

vp_rekey_option
	: KW_SHIFT '(' LL_NUMBER ')' { value_pairs_transform_set_add_func(last_vp_transset, value_pairs_new_transform_shift($3)); }
	| KW_ADD_PREFIX '(' string ')' { value_pairs_transform_set_add_func(last_vp_transset, value_pairs_new_transform_add_prefix($3)); free($3); }
	| KW_REPLACE_PREFIX '(' string string ')' { value_pairs_transform_set_add_func(last_vp_transset, value_pairs_new_transform_replace_prefix($3, $4)); free($3); free($4); }
	;


%%
