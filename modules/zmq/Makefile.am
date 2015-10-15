if ENABLE_ZMQ

module_LTLIBRARIES += modules/zmq/libzmq.la
modules_zmq_libzmq_la_CFLAGS = \
    $(ZMQ_CLIENT_CFLAGS)  \
    $(SYSLOG_NG_CFLAGS) \
    $(EVENTLOG_CFLAGS) \
    $(IVYKIS_CFLAGS) \
    -I$(top_srcdir)/modules/zmq    \
    -I$(top_builddir)/modules/zmq

modules_zmq_libzmq_la_SOURCES = \
    modules/zmq/zmq-grammar.y \
    modules/zmq/zmq-plugin.c \
    modules/zmq/zmq-module.h \
    modules/zmq/zmq-source.c \
    modules/zmq/zmq-destination.c \
    modules/zmq/zmq-parser.c \
    modules/zmq/zmq-parser.h \
    modules/zmq/zmq-transport.c \
    modules/zmq/zmq-transport.h

modules_zmq_libzmq_la_LIBADD = \
    $(ZMQ_CLIENT_LIBS) $(EVENTLOG_LIBS) \
    $(SYSLOG_NG_LIBS) $(IVYKIS_LIBS)

modules_zmq_libzmq_la_LDFLAGS = \
    -avoid-version -module -no-undefined

modules_zmq_libzmq_la_DEPENDENCIES  = \
        $(MODULE_DEPS_LIBS)

include modules/zmq/tests/Makefile.am

modules/zmq modules/zmq/ mod-zmq: modules/zmq/libzmq.la

else
modules/zmq modules/zmq/ mod-zmq:
endif

BUILT_SOURCES += \
    modules/zmq/zmq-grammar.y \
    modules/zmq/zmq-grammar.c \
    modules/zmq/zmq-grammar.h

EXTRA_DIST += \
    modules/zmq/zmq-grammar.ym

.PHONY: modules/zmq/ mod-zmq
