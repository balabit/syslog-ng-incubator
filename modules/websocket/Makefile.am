if ENABLE_WEBSOCKET

module_LTLIBRARIES += modules/websocket/libwebsocket.la
modules_websocket_libwebsocket_la_CFLAGS = \
    $(SYSLOG_NG_CFLAGS) \
	$(LIBWEBSOCKETS_CFLAGS) \
    $(EVENTLOG_CFLAGS) \
    $(IVYKIS_CFLAGS) \
    -I$(top_srcdir)/modules/websocket \
    -I$(top_builddir)/modules/websocket

modules_websocket_libwebsocket_la_SOURCES = \
    modules/websocket/websocket-grammar.y \
    modules/websocket/websocket-plugin.c \
    modules/websocket/websocket-destination.c \
    modules/websocket/websocket-destination.h \
    modules/websocket/websocket-parser.c \
    modules/websocket/websocket-parser.h \
    modules/websocket/ws_api/client.c \
    modules/websocket/ws_api/client.h \
    modules/websocket/ws_api/server.c \
    modules/websocket/ws_api/server.h \
	modules/websocket/websocket-source.c \
	modules/websocket/websocket-source.h \
	modules/websocket/websocket-transport.c \
	modules/websocket/websocket-transport.h

modules_websocket_libwebsocket_la_LIBADD      = \
    $(INCUBATOR_LIBS) $(LIBWEBSOCKETS_LIBS) \
    $(IVYKIS_LIBS) $(EVENTLOG_LIBS)

modules_websocket_libwebsocket_la_LDFLAGS = \
    -avoid-version -module -no-undefined

modules_websocket_libwebsocket_la_DEPENDENCIES  = \
        $(MODULE_DEPS_LIBS)

modules/websocket modules/websocket/ mod-websocket: modules/websocket/libwebsocket.la

else
modules/websocket modules/websocket/ mod-websocket:
endif

BUILT_SOURCES += \
    modules/websocket/websocket-grammar.y \
    modules/websocket/websocket-grammar.c \
    modules/websocket/websocket-grammar.h

EXTRA_DIST += \
    modules/websocket/websocket-grammar.ym

.PHONY: modules/websocket/ mod-websocket
