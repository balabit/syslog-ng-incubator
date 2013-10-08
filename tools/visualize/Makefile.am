if ENABLE_LOGMONGOURCE

bin_PROGRAMS				+= tools/visualize/logmongource

tools_visualize_logmongource_CFLAGS	 = ${LIBMONGO_CFLAGS} ${GLIB_CFLAGS}
tools_visualize_logmongource_LDFLAGS	 = ${LIBMONGO_LIBS} ${GLIB_LIBS} -lm

endif

EXTRA_DIST	+= tools/visualize/README.md
