dnl Process this file with autoconf to produce a configure script.
dnl

AC_PREREQ([2.68])

AC_INIT([syslog-ng-incubator], [0.1.1], [algernon@madhouse-project.org])
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
IVYKIS_MIN_VERSION="0.30.0"
LMC_MIN_VERSION="0.1.2"
LRC_MIN_VERSION="1.0.0"
LUA_MIN_VERSION="5.1.4"

dnl ***************************************************************************
dnl Initial setup

AC_ARG_WITH(module-dir,
   [  --with-module-dir=path   Use path as the list of directories looked up when searching for modules],
   moduledir=$with_module_dir,moduledir="auto")

AC_ARG_WITH(ivykis,
   [  --without-ivykis         Build without explicitly linking to ivykis])

AC_ARG_ENABLE(luajit,
   [  --enable-luajit          Use luajit instead of standard liblua for Lua support],
   enable_luajit="yes",)

dnl ***************************************************************************
dnl Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_CXX
AC_C_INLINE

LT_INIT([shared dlopen])

AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

dnl ***************************************************************************
dnl Miscellanneous headers
dnl ***************************************************************************

AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_FUNCS([floor memset strdup strtoll])

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

if test "x$with_ivykis" != "xno"; then
   PKG_CHECK_MODULES(IVYKIS, ivykis >= $IVYKIS_MIN_VERSION,,true)
fi

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
dnl lua headers/libraries
dnl ***************************************************************************
if test "x$enable_luajit" == "xyes"; then
 PKG_CHECK_EXISTS(luajit, [lua_mod="luajit"; LUA_MIN_VERSION="2.0"])
fi
if test "x$lua_mod" != "xluajit"; then
 PKG_CHECK_EXISTS(lua, lua_mod="lua",
  PKG_CHECK_EXISTS(lua5.2, lua_mod="lua5.2",
   PKG_CHECK_EXISTS(lua5.1, lua_mod="lua5.1")))
fi
PKG_CHECK_MODULES(LUA, $lua_mod >= $LUA_MIN_VERSION,enable_lua="yes",enable_lua="no")

dnl ***************************************************************************
dnl misc features to be enabled
dnl ***************************************************************************

AC_SUBST(syslog_ng_tools)
AC_SUBST(moduledir)

AM_CONDITIONAL(ENABLE_LOGMONGOURCE, [test "$enable_logmongource" != "no"])
AM_CONDITIONAL(ENABLE_RIEMANN, [test "$enable_riemann" != "no"])
AM_CONDITIONAL(ENABLE_LUA, [test "$enable_lua" != "no"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo
echo "syslog-ng Incubator $VERSION configured"
echo "syslog-ng Incubator $VERSION configured" | sed -e "s/./-/g"
echo " Modules:"
echo "  basicfuncs-plus      yes"
echo "  graphite             yes"
echo "  logmongource:        ${enable_logmongource:-yes}"
echo "  lua:                 ${enable_lua:-yes} ${lua_mod:+($lua_mod)}"
echo "  monitor-source:      ${enable_lua:-yes} ${lua_mod:+($lua_mod)}"
echo "  riemann:             ${enable_riemann:-yes}"
echo "  rss                  yes"
echo "  trigger-source       yes"
echo
