modules_zmq_tests_TESTS = \
	modules/zmq/tests/test_zmq

check_PROGRAMS += \
	${modules_zmq_tests_TESTS}

modules_zmq_tests_test_zmq_CFLAGS = \
	$(INCUBATOR_CFLAGS)

modules_zmq_tests_test_zmq_LDADD = \
	$(INCUBATOR_TEST_LDADD) $(INCUBATOR_LIBS) \
	$(top_builddir)/modules/zmq/libzmq.la

include modules/zmq/tests/functional/Makefile.am
