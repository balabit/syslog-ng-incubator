#include <apphook.h>
#include <libtest/testutils.h>

#include "modules/zmq/zmq-module.h"

void
test_source_methods(){
  ZMQSourceDriver *self = g_new0(ZMQSourceDriver, 1);

  zmq_sd_set_address((LogDriver *) self, "example_address");
  zmq_sd_set_port((LogDriver *) self, 42);
  assert_string(zmq_sd_get_address(self), "tcp://example_address:42", "Failed to set zmq address", NULL);
  assert_string(zmq_sd_get_persist_name(self), "zmq_source:example_address:42", "Given persist name is not okay", NULL);
  assert_false(zmq_sd_create_context(self), "Successfully created new zmq context with fake data which is unexpected");

  zmq_sd_set_address((LogDriver *) self, "localhost");
  zmq_sd_set_port((LogDriver *) self, 65530);
  assert_string(zmq_sd_get_address(self), "tcp://localhost:65530", "Failed to set zmq address", NULL);
  assert_true(zmq_sd_create_context(self), "Failed to create zmq context");
}

void
test_destination_methods(){

}


int main()
{
  app_startup();

  test_source_methods();
  test_destination_methods();

  app_shutdown();
  return 0;
}
