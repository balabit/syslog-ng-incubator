dnl Process this file with autoconf to produce a configure script.
dnl

AC_PREREQ([2.69])

AC_INIT([syslog-ng-incubator], [0], [algernon@madhouse-project.org])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz no-dist-gzip])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([no])])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])

dnl ***************************************************************************
dnl definitions

moduledir='${exec_prefix}/lib/syslog-ng'

enable_value()
{
        if test "x$1" = "xyes" ; then
                echo 1
        else
                echo 0
        fi
}

dnl ***************************************************************************
dnl dependencies

GLIB_MIN_VERSION="2.14"
SYSLOG_NG_MIN_VERSION="3.5"
EVENTLOG_MIN_VERSION="0.2.10"
LMC_MIN_VERSION="0.1.2"
LRC_MIN_VERSION="1.0.0"
MYSQL_MIN_VERSION="5.5"

dnl ***************************************************************************
dnl Initial setup

AC_ARG_WITH(module-dir,
   [  --with-module-dir=path   Use path as the list of directories looked up when searching for modules],
   moduledir=$with_module_dir,moduledir="auto")

dnl ***************************************************************************
dnl Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_CXX
AC_C_INLINE

LT_INIT([shared])

AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

dnl ***************************************************************************
dnl Validate yacc
if echo "$YACC" | grep bison > /dev/null; then
  # NOTE: m4 removes [], that's why it needs to be escaped
  bison_version=[`$YACC --version | head -1 | sed -e 's/[^0-9.]*\([-0-9.]\+\)$/\1/'`]
  bison_version_major=`echo $bison_version | cut -d. -f1`
  bison_version_minor=`echo $bison_version | cut -d. -f2`
  if test "$bison_version_major" -lt 2 -o "$bison_version_minor" -lt 4; then
    AC_MSG_WARN([syslog-ng-incubator requires bison 2.4. You may not be able to regenerate parsers if you change them.])
  fi
else
  AC_MSG_WARN([syslog-ng-incubator only supports bison and not traditional yacc. You may not be able to regenerate the parsers if you change them.])
fi

YFLAGS="-d"

dnl ***************************************************************************
dnl Miscellanneous headers
dnl ***************************************************************************

AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_FUNCS([floor memset strdup])

dnl ***************************************************************************
dnl Header checks
dnl ***************************************************************************

dnl ***************************************************************************
dnl Checks for libraries
dnl ***************************************************************************
PKG_CHECK_MODULES(GLIB, glib-2.0 >= $GLIB_MIN_VERSION)

dnl ***************************************************************************
dnl syslog-ng headers/libraries
dnl ***************************************************************************

PKG_CHECK_MODULES(SYSLOG_NG, syslog-ng >= $SYSLOG_NG_MIN_VERSION)
PKG_CHECK_MODULES(EVENTLOG, eventlog >= $EVENTLOG_MIN_VERSION)

syslog_ng_tools=[`pkg-config syslog-ng --variable toolsdir`]
if test "x$moduledir" = "xauto"; then
   moduledir=[`pkg-config syslog-ng --variable moduledir`]
fi

dnl ***************************************************************************
dnl libmongo-client headers/libraries
dnl ***************************************************************************
PKG_CHECK_MODULES(LIBMONGO, libmongo-client >= $LMC_MIN_VERSION,,enable_logmongource="no")

dnl ***************************************************************************
dnl riemann-client headers/libraries
dnl ***************************************************************************
PKG_CHECK_MODULES(RIEMANN_CLIENT, riemann-client >= $LRC_MIN_VERSION,,enable_riemann="no")

dnl ***************************************************************************
dnl mysql-client headers/libraries
dnl ***************************************************************************
AC_CHECK_PROG(enable_mysql, mysql_config, yes, no)
if test "x$enable_mysql" = "xyes"; then
   AC_MSG_CHECKING(for libmysqlclient_r)
   MYSQL_CFLAGS=`mysql_config --cflags`
   MYSQL_LIBS=`mysql_config --libs_r`
   AC_SUBST(MYSQL_CFLAGS)
   AC_SUBST(MYSQL_LIBS)
   AC_MSG_RESULT(yes)
fi

dnl ***************************************************************************
dnl misc features to be enabled
dnl ***************************************************************************

AC_SUBST(syslog_ng_tools)
AC_SUBST(moduledir)

AM_CONDITIONAL(ENABLE_LOGMONGOURCE, [test "$enable_logmongource" != "no"])
AM_CONDITIONAL(ENABLE_RIEMANN, [test "$enable_riemann" != "no"])
AM_CONDITIONAL(ENABLE_MYSQL, [test "$enable_mysql" != "no"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
