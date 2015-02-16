if ENABLE_RUST

export rustmoduledir
export rustmoduledepsdir
export top_srcdir
export top_builddir

module_LTLIBRARIES      += modules/rust/librust.la
modules_rust_librust_la_SOURCES = \
  modules/rust/rust-grammar.y       \
  modules/rust/rust-parser.c        \
  modules/rust/rust-parser.h        \
  modules/rust/rust-plugin.c        \
  modules/rust/rust-filter.c               \
  modules/rust/rust-filter.h

modules_rust_librust_la_CPPFLAGS  =     \
  $(AM_CPPFLAGS)            \
  $(INCUBATOR_CFLAGS)			\
  -I$(top_srcdir)/modules/rust        \
  -I$(top_builddir)/modules/rust	 \
  -I$(top_srcdir)/modules/rust-modules/include
modules_rust_librust_la_LIBADD  = $(MODULE_DEPS_LIBS) $(LIBRUST_MODULES_LIBS)
modules_rust_librust_la_LDFLAGS = $(MODULE_LDFLAGS) -R $(rustmoduledir):$(rustmoduledepsdir)
modules_rust_librust_la_DEPENDENCIES= $(MODULE_DEPS_LIBS)  librust_modules.so


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

