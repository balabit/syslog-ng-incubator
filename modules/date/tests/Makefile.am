modules_date_tests_TESTS = \
	modules/date/tests/test_date

check_PROGRAMS += \
	${modules_date_tests_TESTS}

modules_date_tests_test_date_CFLAGS = \
	$(INCUBATOR_CFLAGS)

modules_date_tests_test_date_LDADD = \
	$(INCUBATOR_TEST_LDADD) $(INCUBATOR_LIBS) \
	$(top_builddir)/modules/date/libdate-parser.la

