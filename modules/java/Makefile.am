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
    modules/java/java-destination-proxy.c \
    modules/java/java-destination-proxy.h \
    modules/java/java-parser.c \
    modules/java/java-parser.h \
    modules/java/java_machine.c \
    modules/java/java_machine.h

modules_java_libmod_java_la_LIBADD = \
    $(JNI_LIBS) $(EVENTLOG_LIBS) \
    $(SYSLOG_NG_LIBS) $(IVYKIS_LIBS)

modules_java_libmod_java_la_LDFLAGS = \
    -avoid-version -module -no-undefined

modules_java_libmod_java_la_DEPENDENCIES  = \
        $(MODULE_DEPS_LIBS)

modules/java modules/java/ mod-java: modules/java/libmod-java.la

clean-local: clean-java-if

$(top_builddir)/modules/java/SyslogNg.h: $(top_builddir)/modules/java/org/syslog_ng/SyslogNgDestination.class
	$(JAVAH) -cp $(top_builddir)/modules/java -o $(top_builddir)/modules/java/SyslogNg.h  org.syslog_ng.SyslogNg


$(top_builddir)/modules/java/org/syslog_ng/SyslogNgDestination.class:
	$(JAVAC) -d $(top_builddir)/modules/java $(top_srcdir)/modules/java/SyslogNgClassLoader.java
	$(JAVAC) -d $(top_builddir)/modules/java $(top_srcdir)/modules/java/SyslogNg.java
	$(JAVAC) -d $(top_builddir)/modules/java -cp $(top_builddir)/modules/java $(top_srcdir)/modules/java/SyslogNgDestination.java
	jar -cvf $(top_builddir)/modules/java/SyslogNg.jar -C $(top_builddir)/modules/java/ org

clean-java-if:
	rm -rf $(top_builddir)/modules/java/org
	rm -rf $(top_builddir)/modules/java/SyslogNg.h
	rm -rf $(top_builddir)/modules/java/SyslogNg.jar

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(moduledir)
	cp $(top_builddir)/modules/java/SyslogNg.jar $(DESTDIR)$(moduledir)/

uninstall-local:
	rm -rf $(DESTDIR)$(moduledir)/SyslogNg.jar

else
modules/java modules/java/ mod-java:
endif

BUILT_SOURCES += \
    modules/java/java-grammar.y \
    modules/java/java-grammar.c \
    modules/java/java-grammar.h \
    modules/java/SyslogNg.h

EXTRA_DIST += \
    modules/java/java-grammar.ym \
    modules/java/SyslogNgClassLoader.java \
    modules/java/SyslogNg.java \
    modules/java/SyslogNgDestination.java

.PHONY: modules/java/ mod-java
