if ENABLE_RUST

export top_srcdir
export top_builddir

lib_LIBRARIES = modules/rust/libsyslog-ng-rust-deps.a
modules_rust_libsyslog_ng_rust_deps_a_SOURCES = \
  modules/rust/rust-grammar.y       \
  modules/rust/rust-parser.c        \
  modules/rust/rust-parser.h        \
  modules/rust/parser.c				\
  modules/rust/parser.h

modules_rust_libsyslog_ng_rust_deps_a_CPPFLAGS = \
  $(INCUBATOR_CFLAGS)	-fPIC		\
  -I$(top_srcdir)/modules/rust        \
  -I$(top_builddir)/modules/rust

modules_rust_libsyslog_ng_rust_deps_a_LDFLAGS = -avoid-version -module -no-undefined

else
modules/rust modules/rust/ mod-rust: modules/rust/librust.la
endif

BUILT_SOURCES       +=      \
  modules/rust/rust-grammar.y       \
  modules/rust/rust-grammar.c       \
  modules/rust/rust-grammar.h

EXTRA_DIST        +=      \
  modules/rust/rust-grammar.ym

.PHONY: modules/rust/ mod-rust

