ACLOCAL_AMFLAGS		= -I m4
AM_YFLAGS		= -Wno-other

check_PROGRAMS		=
TESTS			= ${check_PROGRAMS}
bin_PROGRAMS		=
module_LTLIBRARIES	=

BUILT_SOURCES		=
EXTRA_DIST		= \
	COPYING GPL.txt README.md NEWS ${BUILT_SOURCES}

.y.c:

include modules/Makefile.am
include tools/Makefile.am
include $(syslog_ng_tools)/lex-rules.am
