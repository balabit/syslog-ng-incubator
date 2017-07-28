ACLOCAL_AMFLAGS		= -I m4
AM_YFLAGS		= -Wno-other -d

scldir			= ${datadir}/include/scl

check_PROGRAMS		=
TESTS			= ${check_PROGRAMS}
bin_PROGRAMS		=
module_LTLIBRARIES	=

BUILT_SOURCES		=
EXTRA_DIST		= \
	COPYING GPL.txt README.md NEWS

DISTFILES		= \
	$(filter-out %-grammar.y %-grammar.c %-grammar.h,$(DIST_COMMON)) \
	$(filter-out %-grammar.y,$(DIST_SOURCES))			 \
	$(TEXINFOS) $(EXTRA_DIST)

DISTCLEANFILES		= \
	$(filter %-grammar.y %-grammar.c %-grammar.h,$(DIST_COMMON)) \
	$(filter %-grammar.y,$(DIST_SOURCES))

.y.c:

include modules/Makefile.am
include tools/Makefile.am
include debian/Makefile.am
include $(syslog_ng_tools)/lex-rules.am
