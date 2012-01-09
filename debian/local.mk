## Copyright (C) 2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

PACKAGE_NAME = urbi_$(PACKAGE_VERSION)
DEB_ARCH = `dpkg-architecture -qDEB_BUILD_ARCH`
PREFIX = echo $(prefix) | sed -e "s/\\//\\\\\//g"

.PHONY: packages deb rpm
packages: deb rpm

deb: debian/changelog debian/rules debian/urbi-doc.install debian/urbi.install debian/urbi-dev.install
	$(MAKE) distdir
	rm -rf $(PACKAGE_NAME)
	mv $(distdir) $(PACKAGE_NAME)
	tardir=$(PACKAGE_NAME) && $(am__tar) | bzip2 -9 -c >$(PACKAGE_NAME).tar.bz2
	cp $(PACKAGE_NAME).tar.bz2 $(PACKAGE_NAME).orig.tar.bz2
	cd $(PACKAGE_NAME) && dpkg-buildpackage -j2

rpm:
	fakeroot -- alien -r $(PACKAGE_NAME)_$(DEB_ARCH).deb
	fakeroot -- alien -r urbi-doc_$(PACKAGE_VERSION)_$(DEB_ARCH).deb
	fakeroot -- alien -r urbi-dev_$(PACKAGE_VERSION)_$(DEB_ARCH).deb

.PHONY: debian/changelog
debian/changelog: $(srcdir)/debian/changelog.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]VERSION@/$(PACKAGE_VERSION)/' $< > $@.tmp
	$(AM_V_at)mv $@.tmp $@

.PHONY: debian/rules
debian/rules: $(srcdir)/debian/rules.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]PACKAGE_NAME@/$(PACKAGE_NAME)/' $< > $@.tmp
	$(AM_V_at)mv $@.tmp $@
	chmod +x $(builddir)/debian/rules

.PHONY: debian/urbi-doc.install
debian/urbi-doc.install: $(srcdir)/debian/urbi-doc.install.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]PREFIX@/$(shell $(PREFIX))/' $< > $@.tmp
	$(AM_V_at)mv $@.tmp $@

.PHONY: debian/urbi-dev.install
debian/urbi-dev.install: $(srcdir)/debian/urbi-dev.install.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]PREFIX@/$(shell $(PREFIX))/' $< > $@.tmp
	$(AM_V_at)mv $@.tmp $@

.PHONY: debian/urbi.install
debian/urbi.install: $(srcdir)/debian/urbi.install.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]PREFIX@/$(shell $(PREFIX))/' $< > $@.tmp
	$(AM_V_at)mv $@.tmp $@

EXTRA_DIST +=					\
  debian/README.Debian				\
  debian/README.source				\
  debian/changelog.in				\
  debian/changelog				\
  debian/compat					\
  debian/control				\
  debian/copyright				\
  debian/docs					\
  debian/info					\
  debian/rules.in				\
  debian/rules	         			\
  debian/source/format				\
  debian/urbi-dev.install.in			\
  debian/urbi-dev.install			\
  debian/urbi-doc.docs				\
  debian/urbi-doc.install.in			\
  debian/urbi-doc.install			\
  debian/urbi.install.in			\
  debian/urbi.install
