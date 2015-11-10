#include <apphook.h>
#include <libtest/testutils.h>

#include "modules/zmq/zmq-module.h"

void
test_source_methods(){
  ZMQSourceDriver *self = g_new0(ZMQSourceDriver, 1);

  zmq_sd_set_host((LogDriver *) self, "example_address");
  zmq_sd_set_port((LogDriver *) self, 42);
  assert_string(zmq_sd_get_address(self), "tcp://example_address:42", "Failed to set zmq address");
  assert_string(zmq_sd_get_persist_name(self), "zmq_source:example_address:42", "Given persist name is not okay");
  assert_false(zmq_sd_connect(self), "Connection was successful but it shouldn't have been on source site");

  zmq_sd_set_host((LogDriver *) self, "localhost");
  zmq_sd_set_port((LogDriver *) self, 65530);
  assert_true(zmq_sd_connect(self), "tcp://localhost:65530", "Failed to set zmq address");
}

void
test_destination_methods(){
  ZMQDestDriver *self = g_new0(ZMQDestDriver, 1);

  zmq_dd_set_host((LogDriver *) self, "almafa");
  zmq_dd_set_port((LogDriver *) self, 42);
  assert_string(zmq_dd_get_address(self), "tcp://almafa:42", "The given address is not the expected!");
  assert_false(zmq_dd_connect(self), "Connection was successfully, but it shouldn't have been on source site");

  zmq_dd_set_host((LogDriver *) self, "*");
  zmq_dd_set_port((LogDriver *) self, 64444);
  assert_string(zmq_dd_get_address(self), "tcp://*:64444", "The given address is not the expected!");
  assert_true(zmq_dd_connect(self), "Failed to connect on destination site");
}


int main()
{
  app_startup();

  test_source_methods();
  test_destination_methods();

  app_shutdown();
  return 0;
}
