module_LTLIBRARIES			+= \
  modules/getent/libtfgetent.la

modules_getent_libtfgetent_la_SOURCES	= \
  modules/getent/tfgetent.c

modules_getent_libtfgetent_la_CPPFLAGS	= \
  $(AM_CPPFLAGS)			  \
  $(SYSLOG_NG_CFLAGS)			  \
  $(EVENTLOG_CFLAGS)			  \
  -I$(top_srcdir)/modules/getent
modules_getent_libtfgetent_la_LIBADD	= \
  $(SYSLOG_NG_LIBS) $(EVENTLOG_LIBS)
modules_getent_libtfgetent_la_LDFLAGS	= \
  -avoid-version -module -no-undefined

modules/getent mod-getent: modules/getent/libtfgetent.la

.PHONY: modules/getent mod-getent
