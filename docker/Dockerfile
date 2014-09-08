FROM ubuntu:trusty

MAINTAINER Viktor Tusa <tusavik@gmail.com>

RUN echo "deb http://packages.madhouse-project.org/ubuntu trusty syslog-ng-devel" >> /etc/apt/sources.list
RUN echo "deb http://archive.ubuntu.com/ubuntu trusty universe" >> /etc/apt/sources.list
RUN apt-get update
RUN apt-get -y -V -f install curl
RUN curl -k https://packages.madhouse-project.org/debian/archive-key.txt | apt-key add -
RUN apt-get update
RUN apt-get install -y -V -f syslog-ng-core syslog-ng-dev git automake build-essential libevtlog0 libtool
RUN apt-get install -y -V -f liblua5.2-dev python-dev libperl-dev librdkafka-dev libriemann-client-dev
RUN mkdir /source && git clone https://github.com/balabit/syslog-ng-incubator /source
RUN cd /source && autoreconf -i && mkdir /build && cd /build && /source/configure && make && make install
RUN apt-get clean && rm -Rf /build



