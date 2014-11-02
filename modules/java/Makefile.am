if ENABLE_JAVA

module_LTLIBRARIES += modules/java/libmod-java.la
modules_java_libmod_java_la_CFLAGS = \
    $(JNI_CFLAGS)  \
    $(SYSLOG_NG_CFLAGS) \
    $(EVENTLOG_CFLAGS) \
    $(IVYKIS_CFLAGS) \
    -I$(top_srcdir)/modules/java    \
    -I$(top_builddir)/modules/java

modules_java_libmod_java_la_SOURCES = \
    modules/java/java-grammar.y \
    modules/java/java-plugin.c \
    modules/java/java-destination.c \
    modules/java/java-destination.h \
    modules/java/java-parser.c \
    modules/java/java-parser.h \
    modules/java/java_machine.c \
    modules/java/java_machine.h \
    modules/java/SyslogNgInterface.c

modules_java_libmod_java_la_LIBADD = \
    $(JNI_LIBS) $(EVENTLOG_LIBS) \
    $(SYSLOG_NG_LIBS) $(IVYKIS_LIBS)

modules_java_libmod_java_la_LDFLAGS = \
    -avoid-version -module -no-undefined

modules_java_libmod_java_la_DEPENDENCIES  = \
        $(MODULE_DEPS_LIBS)

modules/java modules/java/ mod-java: modules/java/libmod-java.la

clean-local: clean-java-if

$(top_builddir)/modules/java/SyslogNgInterface.h: $(top_builddir)/modules/java/org/balabit/syslogng/SyslogNgInterface.class
	echo $(moduledir)
	$(JAVAH) -cp $(top_builddir)/modules/java -o $(top_builddir)/modules/java/SyslogNgInterface.h  org.balabit.syslogng.SyslogNgInterface


$(top_builddir)/modules/java/org/balabit/syslogng/SyslogNgInterface.class:
	mkdir -p $(top_builddir)/modules/java/org/balabit/syslogng
	$(JAVAC) -d $(top_builddir)/modules/java $(top_srcdir)/modules/java/SyslogNgInterface.java

clean-java-if:
	rm -rf $(top_builddir)/modules/java/org
	rm -rf $(top_builddir)/modules/java/SyslogNgInterface.h

install-exec-local:
	jar -cvf $(moduledir)/SyslogNgInterface.jar -C modules/java/ org

else
modules/java modules/java/ mod-java:
endif

BUILT_SOURCES += \
    modules/java/java-grammar.y \
    modules/java/java-grammar.c \
    modules/java/java-grammar.h \
    modules/java/SyslogNgInterface.h

EXTRA_DIST += \
    modules/java/java-grammar.ym

.PHONY: modules/java/ mod-java
