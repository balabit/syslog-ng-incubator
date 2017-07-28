#! /bin/sh
top_srcdir=$(cd ${top_srcdir}; pwd)
top_builddir=$(cd ${top_builddir}; pwd)
export func_test_dir=$(cd ${top_srcdir}/syslog-ng/tests/functional; pwd)


if [ -z "$SYSLOG_NG_BINARY" ]
then
  SYSLOG_NG_BINARY=$(which syslog-ng)
  if [ -z "$SYSLOG_NG_BINARY" ]
  then
    echo "Failed to determine syslog-ng binary."
    exit 1
  fi
fi

export SYSLOG_NG_BINARY

${top_srcdir}/modules/zmq/tests/functional/test_zmq_source.py
${top_srcdir}/modules/zmq/tests/functional/test_zmq_destination.py
