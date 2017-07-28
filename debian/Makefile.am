EXTRA_DIST		+= \
			debian/compat \
			debian/changelog debian/changelog.in \
			debian/syslog-ng-incubator-mods.install \
			debian/syslog-ng-incubator-mods.triggers \
			debian/rules debian/control

debian/changelog: debian/changelog.in
	$(top_builddir)/config.status --file $@:$<

MAINTAINERCLEANFILES	= debian/changelog
