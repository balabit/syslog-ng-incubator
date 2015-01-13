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

JAVA_PKG_NAME = org.syslog_ng

JAVA_CLASS_DIR = org/syslog_ng

JAVA_CLASS_PATH = $(top_builddir)/modules/java/$(JAVA_CLASS_DIR)
JAVA_SRC_DIR = $(top_srcdir)/modules/java

JAVA_CLASS_FILES = \
    $(JAVA_CLASS_PATH)/SyslogNgClassLoader.class \
    $(JAVA_CLASS_PATH)/LogMessage.class \
    $(JAVA_CLASS_PATH)/LogTemplate.class \
    $(JAVA_CLASS_PATH)/LogPipe.class \
    $(JAVA_CLASS_PATH)/LogDestination.class \
    $(JAVA_CLASS_PATH)/TextLogDestination.class \
    $(JAVA_CLASS_PATH)/StructuredLogDestination.class \
    $(JAVA_CLASS_PATH)/DummyTextDestination.class

JAVA_FILES = \
    $(JAVA_SRC_DIR)/SyslogNgClassLoader.java \
    $(JAVA_SRC_DIR)/LogMessage.java \
    $(JAVA_SRC_DIR)/LogTemplate.java \
    $(JAVA_SRC_DIR)/LogPipe.java \
    $(JAVA_SRC_DIR)/LogDestination.java \
    $(JAVA_SRC_DIR)/TextLogDestination.java \
    $(JAVA_SRC_DIR)/StructuredLogDestination.java \
    $(JAVA_SRC_DIR)/DummyTextDestination.java

JAVA_HEADER_FILES = \
   $(top_builddir)/modules/java/LogDestination.h \
   $(top_builddir)/modules/java/LogMessage.h \
   $(top_builddir)/modules/java/LogTemplate.h


modules/java modules/java/ mod-java: modules/java/libmod-java.la

modules/java/SyslogNg.jar: $(JAVA_CLASS_FILES) $(JAVA_HEADER_FILES)
	$(AM_V_GEN) $(JAR) -cvf $@ -C $(top_builddir)/modules/java/ org

$(JAVA_HEADER_FILES): $(JAVA_CLASS_FILES)
	$(AM_V_GEN) $(JAVAH) -cp $(top_builddir)/modules/java -o $@ $(JAVA_PKG_NAME).$(notdir $(basename $@))

$(JAVA_CLASS_PATH)/%.class: $(top_srcdir)/modules/java/%.java $(JAVA_FILES)
	$(AM_V_GEN) $(JAVAC) -d $(top_builddir)/modules/java -cp $(top_builddir)/modules/java $<

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(moduledir)
	cp modules/java/SyslogNg.jar $(DESTDIR)$(moduledir)/

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
BUILT_SOURCES += $(JAVA_HEADER_FILES) \
                 modules/java/SyslogNg.jar

CLEANFILES = $(JAVA_CLASS_FILES) \
             $(JAVA_HEADER_FILES) \
             modules/java/SyslogNg.jar
endif

EXTRA_DIST += \
    modules/java/native/java-grammar.ym \
    $(JAVA_FILES)

.PHONY: modules/java/ mod-java
