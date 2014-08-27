module_LTLIBRARIES			+= \
  modules/getent/libtfgetent.la

modules_getent_libtfgetent_la_SOURCES	= \
  modules/getent/tfgetent.c

EXTRA_DIST				+=\
  modules/getent/getent-protocols.c	  \
  modules/getent/getent-services.c	  \
  modules/getent/getent-group.c	  \
  modules/getent/getent-passwd.c

modules_getent_libtfgetent_la_CPPFLAGS	= \
  $(AM_CPPFLAGS)			  \
  $(INCUBATOR_CFLAGS)			  \
  -I$(top_srcdir)/modules/getent
modules_getent_libtfgetent_la_LIBADD	= \
  $(INCUBATOR_LIBS)
modules_getent_libtfgetent_la_LDFLAGS	= \
  -avoid-version -module -no-undefined

modules/getent mod-getent: modules/getent/libtfgetent.la

.PHONY: modules/getent mod-getent
