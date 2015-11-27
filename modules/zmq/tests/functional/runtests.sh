#! /bin/sh
set -e

top_srcdir=$(cd ${top_srcdir}; pwd)
top_builddir=$(cd ${top_builddir}; pwd)
export func_test_dir=$(cd ${top_srcdir}/syslog-ng/tests/functional; pwd)
export SYSLOG_NG_BINARY=$(which syslog-ng)

${top_srcdir}/modules/zmq/tests/functional/test_zmq_source.py
${top_srcdir}/modules/zmq/tests/functional/test_zmq_destination.py
