#include <apphook.h>
#include <libtest/testutils.h>

#include "modules/zmq/zmq-module.h"

void
test_source_methods(){
  ZMQSourceDriver *self = g_new0(ZMQSourceDriver, 1);
  zmq_sd_set_address((LogDriver *) self, "example_address");
  zmq_sd_set_port((LogDriver *) self, 42);
  assert_string(self->address, "example_address", "Failed to set proper address of zmq source", NULL);
  assert_gint(self->port, 42, "Failed to set address of zmq source", NULL);
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
