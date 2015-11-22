export top_srcdir
export top_builddir

EXTRA_DIST	+= \
		tests/functional/runtests.sh \
		tests/functional/func_test.py

func-test:
	@$(top_srcdir)/modules/zmq/tests/functional/runtests.sh

.PHONY: func-test
