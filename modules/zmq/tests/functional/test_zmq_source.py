#!/usr/bin/env python

from control import *

conf = """
@version: 3.8

source zmq {
    zmqq();
};

destination file {
    file("/tmp/res");
};

log {
    source(zmq);
    destination(file);
};
"""

def test_zmq_source_can_receive_a_message():
    if not start_syslogng(conf, keep_persist=False, verbose=False):
        sys.exit(42)
    stop_syslogng()
