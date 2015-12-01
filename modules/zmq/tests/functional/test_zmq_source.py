#!/usr/bin/env python

import os
import sys

sys.path.append(os.environ['func_test_dir'])

from control import *
from func_test import *

conf = """
@version: 3.7

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

test_zmq_source_can_receive_a_message()
