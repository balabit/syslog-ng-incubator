#!/bin/bash

wget https://github.com/zeromq/libzmq/archive/master.zip -O /tmp/master.zip

cd /tmp
unzip master.zip
mkdir $HOME/install/libzmq
cd libzmq-master && ./autogen.sh && ./configure --prefix=$HOME/install/libzmq && make && make install
