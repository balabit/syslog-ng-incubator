export top_srcdir
export abs_top_srcdir
export abs_top_builddir
export rustmoduledir
export rustmoduledepsdir
export rust_cargo_build_options
export rust_cargo_build_dir
export SYSLOG_NG_LIBDIR

all-local: librust_modules.so

librust_modules.so:
	ln -sf $(abs_top_srcdir)/modules/rust-modules/$(rust_cargo_build_dir) $(abs_top_builddir)/modules/rust-modules
	cd $(top_srcdir)/modules/rust-modules && cargo build -v $(rust_cargo_build_options)
	cd $(abs_top_builddir)/modules/rust-modules/ && ln -sf librust_modules-*.so librust_modules.so

rust-modules-install-exec-hook:
	mkdir -p $(rustmoduledir)
	mkdir -p $(rustmoduledepsdir)
	cp $(abs_top_builddir)/modules/rust-modules/*.so $(rustmoduledir)/
	cp $(abs_top_builddir)/modules/rust-modules/deps/*.so $(rustmoduledepsdir)/

rust-modules-clean-hook:
	cd $(top_srcdir)/modules/rust-modules && cargo clean
	rm -rf $(rustmoduledir)/*
	rm -f $(abs_top_builddir)/modules/rust-modules

INSTALL_EXEC_HOOKS += rust-modules-install-exec-hook
CLEAN_HOOKS += rust-modules-clean-hook

.PHONY: all-local clean-local
