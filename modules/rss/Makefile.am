module_LTLIBRARIES              += modules/rss/librss.la

modules_rss_librss_la_CFLAGS		= \
	$(SYSLOG_NG_CFLAGS)		  \
	$(EVENTLOG_CFLAGS)		  \
	$(IVYKIS_CFLAGS)		  \
	-I$(top_srcdir)/modules/rss	  \
	-I$(top_builddir)/modules/rss

modules_rss_librss_la_SOURCES		= \
	modules/rss/rss-grammar.y	  \
	modules/rss/rss.c		  \
	modules/rss/rss.h		  \
	modules/rss/rss-parser.c	  \
	modules/rss/rss-parser.h

modules_rss_librss_la_LDFLAGS		= \
	-avoid-version -module -no-undefined

BUILT_SOURCES			       += \
	modules/rss/rss-grammar.y	  \
	modules/rss/rss-grammar.c	  \
	modules/rss/rss-grammar.h

modules/rss mod-rss: modules/rss/librss.la

EXTRA_DIST			       += \
	modules/rss/rss-grammar.ym	  \
	modules/rss/rss-example.conf

.PHONY: modules/rss mod-rss
