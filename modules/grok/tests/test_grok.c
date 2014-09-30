#include "modules/grok/grok-parser.h"
#include <apphook.h>
#include <libtest/testutils.h>

LogParser*
create_simple_parser()
{
   LogParser *parser = grok_parser_new();
   grok_parser_turn_on_debug(parser);
   grok_parser_add_named_subpattern(parser, "STRING", "[a-zA-Z]*");
   return parser; 
};

void
test_grok_pattern()
{
   LogParser *parser = create_simple_parser(); 

   GrokInstance *instance = grok_instance_new();
   grok_instance_set_pattern(instance, "%{STRING:field}");
   grok_parser_add_pattern_instance(parser, instance);
   
   LogMessage *msg = log_msg_new_empty();
   NVHandle handle = LM_V_MESSAGE;
   log_msg_set_value(msg, handle, "value", sizeof("value"));

   LogPathOptions options;
   
   log_pipe_init(parser, NULL);
   
   log_parser_process(parser, &msg, &options, NULL, 0);

   NVHandle field = log_msg_get_value_handle("field");
   gssize value_len;
   gchar *value = log_msg_get_value(msg, field, &value_len);
   
   assert_nstring(value, value_len, "value", 5, "Named capture didn't stored");
   
};

int main()
{
  app_startup();
  test_grok_pattern();
  app_shutdown();
  return 0;
};
