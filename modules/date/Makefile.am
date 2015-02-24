if ENABLE_DATE
module_LTLIBRARIES += modules/date/libdate-parser.la

modules_date_libdate_parser_la_CFLAGS	 = \
	$(INCUBATOR_CFLAGS)			   \
	-I$(top_srcdir)/modules/date		   \
	-I$(top_builddir)/modules/date

modules_date_libdate_parser_la_SOURCES	 = \
	modules/date/date-parser-grammar.y	   \
	modules/date/date-parser.c		   \
	modules/date/date-parser.h		   \
	modules/date/date-parser-parser.c	   \
	modules/date/date-parser-parser.h	   \
	modules/date/date-parser-plugin.c

modules_date_libdate_parser_la_LIBADD	 = \
	$(INCUBATOR_LIBS)

modules_date_libdate_parser_la_LDFLAGS	 = \
	-avoid-version -module -no-undefined

BUILT_SOURCES						+= \
	modules/date/date-parser-grammar.y	   \
	modules/date/date-parser-grammar.c	   \
	modules/date/date-parser-grammar.h

include modules/date/tests/Makefile.am

modules/date-parser mod-date: modules/date/libdate-parser.la
else
modules/date-parser mod-date:
endif


EXTRA_DIST += \
	modules/date/date-parser-grammar.ym

.PHONY: modules/date mod-date
