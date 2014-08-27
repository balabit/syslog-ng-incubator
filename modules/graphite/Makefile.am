module_LTLIBRARIES          += \
  modules/graphite/libgraphite.la

modules_graphite_libgraphite_la_SOURCES  = \
  modules/graphite/graphite-plugin.c  \
  modules/graphite/graphite-output.h  \
  modules/graphite/graphite-output.c

modules_graphite_libgraphite_la_CPPFLAGS   = \
  $(AM_CPPFLAGS)             \
  $(INCUBATOR_CFLAGS)          \
  -I$(top_srcdir)/modules/graphite
modules_graphite_libgraphite_la_LIBADD   = \
  $(INCUBATOR_LIBS)
modules_graphite_libgraphite_la_LDFLAGS  = \
  -avoid-version -module -no-undefined

modules/graphite mod-graphite: modules/graphite/libgraphite.la

.PHONY: modules/graphite mod-graphite

