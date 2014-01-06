module_LTLIBRARIES					+= modules/trigger-source/libtrigger-source.la

modules_trigger_source_libtrigger_source_la_CFLAGS	 = \
	$(SYSLOG_NG_CFLAGS)				   \
	$(EVENTLOG_CFLAGS)				   \
	$(IVYKIS_CFLAGS)				   \
	-I$(top_srcdir)/modules/trigger-source		   \
	-I$(top_builddir)/modules/trigger-source

modules_trigger_source_libtrigger_source_la_SOURCES	 = \
	modules/trigger-source/trigger-source.c		   \
	modules/trigger-source/trigger-source.h		   \
	modules/trigger-source/trigger-source-parser.c	   \
	modules/trigger-source/trigger-source-parser.h	   \
	modules/trigger-source/trigger-source-plugin.c

nodist_modules_trigger_source_libtrigger_source_la_SOURCES=\
	modules/trigger-source/trigger-source-grammar.y	   \
	modules/trigger-source/trigger-source-grammar.c	   \
	modules/trigger-source/trigger-source-grammar.h

modules_trigger_source_libtrigger_source_la_LIBADD	 = \
	$(EVENTLOG_LIBS)				   \
	$(SYSLOG_NG_LIBS)				   \
	$(IVYKIS_LIBS)

modules_trigger_source_libtrigger_source_la_LDFLAGS	 = \
	-avoid-version -module -no-undefined

modules/trigger-source mod-trigger: modules/trigger-source/libtrigger-source.la

EXTRA_DIST						+= \
	modules/trigger-source/trigger-source-grammar.ym	   \
	modules/trigger-source/trigger-source-example.conf

.PHONY: modules/trigger-source mod-trigger
