#!/usr/bin/env python

import time
import zmq

def producer():
    context = zmq.Context()
    zmq_socket = context.socket(zmq.PUSH)
    zmq_socket.bind("tcp://127.0.0.1:5558")
    for num in xrange(1):
        zmq_socket.send("Almafa!\n")

producer()
