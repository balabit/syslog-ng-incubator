if ENABLE_JAVA

module_LTLIBRARIES += modules/java/libmod-java.la
modules_java_libmod_java_la_CFLAGS = \
    $(JNI_CFLAGS)  \
    $(INCUBATOR_CFLAGS) \
    -I$(top_srcdir)/modules/java    \
    -I$(top_builddir)/modules/java	\
    -I$(top_srcdir)/modules/java/native \
    -I$(top_builddir)/modules/java/native

modules_java_libmod_java_la_SOURCES = \
    modules/java/native/java-grammar.y \
    modules/java/native/java-plugin.c \
    modules/java/native/java-destination.c \
    modules/java/native/java-destination.h \
    modules/java/proxies/java-destination-proxy.c \
    modules/java/proxies/java-destination-proxy.h \
    modules/java/proxies/java-logmsg-proxy.c \
    modules/java/proxies/java-logmsg-proxy.h \
    modules/java/proxies/java-template-proxy.c \
    modules/java/proxies/java-template-proxy.h \
    modules/java/native/java-class-loader.c \
    modules/java/native/java-class-loader.h \
    modules/java/native/java-parser.c \
    modules/java/native/java-parser.h \
    modules/java/native/java_machine.c \
    modules/java/native/java_machine.h

modules_java_libmod_java_la_LIBADD = \
    $(JNI_LIBS) $(INCUBATOR_LIBS)

modules_java_libmod_java_la_LDFLAGS = \
    -avoid-version -module -no-undefined

modules_java_libmod_java_la_DEPENDENCIES  = \
        $(MODULE_DEPS_LIBS)

modules/java modules/java/ mod-java: modules/java/libmod-java.la

clean-local: clean-java-if

$(top_builddir)/modules/java/LogDestination.h: $(top_builddir)/modules/java/org/syslog_ng/LogDestination.class
	$(JAVAH) -cp $(top_builddir)/modules/java -o $(top_builddir)/modules/java/LogDestination.h  org.syslog_ng.LogDestination
	$(JAVAH) -cp $(top_builddir)/modules/java -o $(top_builddir)/modules/java/LogMessage.h  org.syslog_ng.LogMessage
	$(JAVAH) -cp $(top_builddir)/modules/java -o $(top_builddir)/modules/java/LogTemplate.h  org.syslog_ng.LogTemplate


$(top_builddir)/modules/java/org/syslog_ng/LogDestination.class:
	$(JAVAC) -d $(top_builddir)/modules/java $(top_srcdir)/modules/java/SyslogNgClassLoader.java
	$(JAVAC) -d $(top_builddir)/modules/java -cp $(top_builddir)/modules/java $(top_srcdir)/modules/java/LogMessage.java
	$(JAVAC) -d $(top_builddir)/modules/java -cp $(top_builddir)/modules/java $(top_srcdir)/modules/java/LogTemplate.java
	$(JAVAC) -d $(top_builddir)/modules/java -cp $(top_builddir)/modules/java $(top_srcdir)/modules/java/LogDestination.java
	$(JAVAC) -d $(top_builddir)/modules/java -cp $(top_builddir)/modules/java $(top_srcdir)/modules/java/TextLogDestination.java
	$(JAVAC) -d $(top_builddir)/modules/java -cp $(top_builddir)/modules/java $(top_srcdir)/modules/java/StructuredLogDestination.java
	jar -cvf $(top_builddir)/modules/java/SyslogNg.jar -C $(top_builddir)/modules/java/ org

clean-java-if:
	rm -rf $(top_builddir)/modules/java/org
	rm -rf $(top_builddir)/modules/java/LogDestination.h
	rm -rf $(top_builddir)/modules/java/LogMessage.h
	rm -rf $(top_builddir)/modules/java/LogTemplate.h
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
    modules/java/native/java-grammar.y \
    modules/java/native/java-grammar.c \
    modules/java/native/java-grammar.h

if ENABLE_JAVA
BUILT_SOURCES += modules/java/LogDestination.h \
                 modules/java/LogMessage.h \
                 modules/java/LogTemplate.h
endif

EXTRA_DIST += \
    modules/java/native/java-grammar.ym \
    modules/java/SyslogNgClassLoader.java \
    modules/java/LogDestination.java \
    modules/java/TextLogDestination.java \
    modules/java/StructuredLogDestination.java \
    modules/java/LogMessage.java \
    modules/java/LogTemplate.java

.PHONY: modules/java/ mod-java
