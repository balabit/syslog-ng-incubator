if ENABLE_JAVA

module_LTLIBRARIES += modules/java/libjava.la
modules_java_libjava_la_CFLAGS = \
    $(JNI_CFLAGS)  \
    $(SYSLOG_NG_CFLAGS) \
    $(EVENTLOG_CFLAGS) \
    $(IVYKIS_CFLAGS) \
    -I$(top_srcdir)/modules/java    \
    -I$(top_builddir)/modules/java \
    -Wall -Werror

modules_java_libjava_la_SOURCES = \
    modules/java/java-grammar.y \
    modules/java/java-plugin.c \
    modules/java/java-destination.c \
    modules/java/java-destination.h \
    modules/java/java-parser.c \
    modules/java/java-parser.h \
    modules/java/java_machine.c \
    modules/java/java_machine.h

modules_java_libjava_la_LIBADD = \
    $(JNI_LIBS) $(EVENTLOG_LIBS) \
    $(SYSLOG_NG_LIBS) $(IVYKIS_LIBS)

modules_java_libjava_la_LDFLAGS = \
    -avoid-version -module -no-undefined

modules_java_libjava_la_DEPENDENCIES  = \
        $(MODULE_DEPS_LIBS)

modules/java modules/java/ mod-java: modules/java/libjava.la

else
modules/java modules/java/ mod-java:
endif

BUILT_SOURCES += \
    modules/java/java-grammar.y \
    modules/java/java-grammar.c \
    modules/java/java-grammar.h

EXTRA_DIST += \
    modules/java/java-grammar.ym

.PHONY: modules/java/ mod-java
