#if ENABLE_GROK
module_LTLIBRARIES					+= modules/grok/libgrok-parser.la

modules_grok_libgrok_parser_la_CFLAGS	 = \
	$(SYSLOG_NG_CFLAGS)				   \
	$(EVENTLOG_CFLAGS)				   \
	$(IVYKIS_CFLAGS)				   \
	-I$(top_srcdir)/modules/grok		   \
	-I$(top_builddir)/modules/grok

modules_grok_libgrok_parser_la_SOURCES	 = \
	modules/grok/grok-parser-grammar.y	   \
	modules/grok/grok-parser.c		   \
	modules/grok/grok-parser.h		   \
	modules/grok/grok-parser-parser.c	   \
	modules/grok/grok-parser-parser.h	   \
	modules/grok/grok-parser-plugin.c

modules_grok_libgrok_parser_la_LIBADD	 = \
	$(EVENTLOG_LIBS)				   \
	$(SYSLOG_NG_LIBS)				   \
	$(IVYKIS_LIBS)

modules_grok_libgrok_parser_la_LDFLAGS	 = \
	-avoid-version -module -no-undefined  $(GROK_LIBS)

BUILT_SOURCES						+= \
	modules/grok/grok-parser-grammar.y	   \
	modules/grok/grok-parser-grammar.c	   \
	modules/grok/grok-parser-grammar.h


modules/grok-parser mod-grok: modules/grok/libgrok-parser.la
#else
modules/grok-parser mod-grok:
#endif


EXTRA_DIST						+= \
	modules/grok/grok-parser-grammar.ym	   \
	modules/grok/grok-parser-example.conf

.PHONY: modules/grok mod-grok

include modules/grok/tests/Makefile.am
