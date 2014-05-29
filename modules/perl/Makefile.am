if ENABLE_PERL

module_LTLIBRARIES				+= modules/perl/libmod-perl.la

PERL_CFLAGS	= $(shell perl -MExtUtils::Embed -e ccopts)
PERL_LDADD	= $(shell perl -MExtUtils::Embed -e ldopts)

modules_perl_libmod_perl_la_CFLAGS		= \
	$(PERL_CFLAGS)				  \
	$(SYSLOG_NG_CFLAGS)			  \
	$(EVENTLOG_CFLAGS)			  \
	$(IVYKIS_CFLAGS)			  \
	-I$(top_srcdir)/modules/perl		  \
	-I$(top_builddir)/modules/perl

modules_perl_libmod_perl_la_LIBADD		= \
	$(EVENTLOG_LIBS)			  \
	$(SYSLOG_NG_LIBS)			  \
	$(IVYKIS_LIBS)

modules_perl_libmod_perl_la_SOURCES		= \
	modules/perl/perl-plugin.c		  \
	modules/perl/perl-grammar.y		  \
	modules/perl/perl-dest.c		  \
	modules/perl/perl-dest.h		  \
	modules/perl/perl-parser.c		  \
	modules/perl/perl-parser.h

modules_perl_libmod_perl_la_LDFLAGS		 = \
	-avoid-version -module -no-undefined	   \
	$(PERL_LDADD)

BUILT_SOURCES					+= \
	modules/perl/perl-grammar.y		   \
	modules/perl/perl-grammar.c		   \
	modules/perl/perl-grammar.h

modules/perl mod-perl: modules/perl/libmod-perl.la
else
modules/perl mod-perl:
endif

EXTRA_DIST					+= \
	modules/perl/perl-grammar.ym		   \
	modules/perl/perl-example.conf		   \
	modules/perl/perl-example.pl

.PHONY: modules/perl mod-perl
