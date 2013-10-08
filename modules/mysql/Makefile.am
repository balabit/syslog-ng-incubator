if ENABLE_MYSQL

module_LTLIBRARIES			+= \
	modules/mysql/libmysql-mod.la

modules_mysql_libmysql_mod_la_CFLAGS	 = \
	$(SYSLOG_NG_CFLAGS)		   \
	$(MYSQL_CFLAGS)			   \
	$(EVENTLOG_CFLAGS)		   \
	-I$(top_srcdir)/modules/mysql	   \
	-I$(top_builddir)/modules/mysql

modules_mysql_libmysql_mod_la_SOURCES	 = \
	modules/mysql/mysql-mod-grammar.y  \
	modules/mysql/mysql-mod.c	   \
	modules/mysql/mysql-mod.h	   \
	modules/mysql/mysql-mod-parser.c   \
	modules/mysql/mysql-mod-parser.h   \
	modules/mysql/mysql-mod-plugin.c

modules_mysql_libmysql_mod_la_LIBADD	 = \
	$(MYSQL_LIBS)			   \
	$(SYSLOG_NG_LIBS)		   \
	$(EVENTLOG_LIBS)

modules_mysql_libmysql_mod_la_LDFLAGS	 = \
	-avoid-version -module -no-undefined

modules/mysql mod-mysql: modules/mysql/libmysql-mod.la
else
modules/mysql mod-mysql:
endif

BUILT_SOURCES				+= \
	modules/mysql/mysql-mod-grammar.y   \
	modules/mysql/mysql-mod-grammar.c   \
	modules/mysql/mysql-mod-grammar.h

EXTRA_DIST				+= \
	modules/mysql/mysql-mod-grammar.ym \
	modules/mysql/mysql-example.conf

.PHONY: modules/mysql mod-mysql
