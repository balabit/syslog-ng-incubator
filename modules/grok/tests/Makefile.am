modules_grok_tests_TESTS = \
	modules/grok/tests/test_grok

check_PROGRAMS += \
	${modules_grok_tests_TESTS}

modules_grok_tests_test_grok_CFLAGS = \
	$(INCUBATOR_CFLAGS)

modules_grok_tests_test_grok_LDADD = \
	$(INCUBATOR_TEST_LDADD) $(INCUBATOR_LIBS) \
	$(top_builddir)/modules/grok/libgrok-parser.la

