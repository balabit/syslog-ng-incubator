#include "modules/grok/grok-parser.h"
#include <apphook.h>
#include <libtest/testutils.h>

LogParser*
create_simple_parser()
{
   LogParser *parser = grok_parser_new();
   grok_parser_turn_on_debug(parser);
   grok_parser_add_named_subpattern(parser, "STRING", "[a-zA-Z]+");
   grok_parser_add_named_subpattern(parser, "NUMBER", "[0-9]+");
   return parser; 
};

LogMessage *
create_message_with_fields(const char *field_name, ...)
{
  va_list args;
  const char* arg;
  LogMessage *msg = log_msg_new_empty();
  arg = field_name;
  va_start(args, field_name);
  while (arg != NULL)
    {   
      NVHandle handle = log_msg_get_value_handle(arg);
      arg = va_arg(args, char*);
      log_msg_set_value(msg, handle, arg, -1);
      arg = va_arg(args, char*);
    }   
  va_end(args);
  return msg;
}

void
parse_msg_with_defaults(LogParser *parser, LogMessage *msg)
{
   LogPathOptions options;

   log_pipe_init(parser, NULL);
   log_parser_process(parser, &msg, &options, NULL, 0);
   log_pipe_deinit(parser);
}

void
create_and_add_grok_instance_with_pattern(LogParser *parser, const char* pattern)
{
   GrokInstance *instance = grok_instance_new();
   grok_instance_set_pattern(instance, pattern);
   grok_parser_add_pattern_instance(parser, instance);
};

void
test_grok_pattern_single()
{
   LogParser *parser = create_simple_parser(); 
   create_and_add_grok_instance_with_pattern(parser, "%{STRING:field}");

   LogMessage *msg = create_message_with_fields("MESSAGE", "value", NULL);
    
   parse_msg_with_defaults(parser, msg);

   NVHandle field = log_msg_get_value_handle("field");
   gssize value_len;
   gchar *value = log_msg_get_value(msg, field, &value_len);
   
   assert_nstring(value, value_len, "value", 5, "Named capture didn't stored");
   log_pipe_unref(parser);
}

void
test_grok_pattern_multiple()
{
   LogParser *parser = create_simple_parser(); 
   create_and_add_grok_instance_with_pattern(parser, "%{STRING:field}");
   create_and_add_grok_instance_with_pattern(parser, "%{NUMBER:field2}");
   
   LogMessage *msg = create_message_with_fields("MESSAGE", "123", NULL);
  
   parse_msg_with_defaults(parser, msg);

   NVHandle field = log_msg_get_value_handle("field2");
   gssize value_len;
   gchar *value = log_msg_get_value(msg, field, &value_len);
   
   assert_nstring(value, value_len, "123", 3, "Named capture didn't stored");
   log_pipe_unref(parser);
} 

void test_grok_parser_clone()
{
   LogParser *old_parser = create_simple_parser(); 
   create_and_add_grok_instance_with_pattern(old_parser, "%{STRING:field}");

   LogParser *parser = log_pipe_clone(&old_parser->super);
   LogMessage *msg = create_message_with_fields("MESSAGE", "value", NULL);
    
   parse_msg_with_defaults(parser, msg);

   NVHandle field = log_msg_get_value_handle("field");
   gssize value_len;
   gchar *value = log_msg_get_value(msg, field, &value_len);
   
   assert_nstring(value, value_len, "value", 5, "Named capture didn't stored");
   log_pipe_unref(old_parser);
   log_pipe_unref(parser);
};

int main()
{
  app_startup();
  test_grok_pattern_single();
  test_grok_pattern_multiple();
  test_grok_parser_clone();
  app_shutdown();
  return 0;
};
