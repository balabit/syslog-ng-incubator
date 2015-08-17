module_LTLIBRARIES					+= \
	modules/basicfuncs-plus/libbasicfuncs-plus.la

modules_basicfuncs_plus_libbasicfuncs_plus_la_SOURCES	 = \
	modules/basicfuncs-plus/basic-funcs-plus.c
modules_basicfuncs_plus_libbasicfuncs_plus_la_CPPFLAGS	 = \
	$(AM_CPPFLAGS)					   \
	$(INCUBATOR_CFLAGS)				   \
	-I$(top_srcdir)/modules/basicfuncs-plus
modules_basicfuncs_plus_libbasicfuncs_plus_la_LIBADD	 = \
	$(INCUBATOR_LIBS)
modules_basicfuncs_plus_libbasicfuncs_plus_la_LDFLAGS	 = \
	-avoid-version -module -no-undefined

EXTRA_DIST						+= \
	modules/basicfuncs-plus/number-funcs.c		   \
	modules/basicfuncs-plus/state-funcs.c

modules/basicfuncs-plus mod-basicfuncs-plus: modules/basicfuncs-plus/libbasicfuncs-plus.la

.PHONY: modules/basicfuncs-plus mod-basicfuncs-plus
