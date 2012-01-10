## Copyright (C) 2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

URBI_VERSION_SHA1 = $(shell echo $(VERSION) | sed -e "s/-/./g")
URBI_VERSION = $(shell echo $(VERSION) | sed -e "s/-/./g")
DIR_NAME = urbi_$(URBI_VERSION)
PACKAGE_NAME = urbi_$(URBI_VERSION_SHA1)
DEB_ARCH = `dpkg-architecture -qDEB_BUILD_ARCH`
PREFIX = $(shell echo $(prefix) | sed -e "s/\\//\\\\\//g")

.PHONY: packages deb rpm
packages: deb rpm

deb: debian/changelog debian/rules debian/urbi-doc.install debian/urbi.install debian/urbi-dev.install
	$(MAKE) distdir
	rm -rf $(DIR_NAME)
	mv $(distdir) $(DIR_NAME)
	tardir=$(DIR_NAME) && $(am__tar) | bzip2 -9 -c >$(DIR_NAME).tar.bz2
	cp $(DIR_NAME).tar.bz2 $(DIR_NAME).orig.tar.bz2
	cd $(DIR_NAME) && dpkg-buildpackage -j2

rpm:
	fakeroot -- alien -r urbi_$(URBI_VERSION_SHA1)_$(DEB_ARCH).deb
	fakeroot -- alien -r urbi-doc_$(URBI_VERSION_SHA1)_$(DEB_ARCH).deb
	fakeroot -- alien -r urbi-dev_$(URBI_VERSION_SHA1)_$(DEB_ARCH).deb
	mv urbi-$(URBI_VERSION_SHA1)-2.$(DEB_ARCH).rpm urbi-$(URBI_VERSION_SHA1)_$(DEB_ARCH).rpm
	mv urbi-doc-$(URBI_VERSION_SHA1)-2.$(DEB_ARCH).rpm urbi-doc-$(URBI_VERSION_SHA1)_$(DEB_ARCH).rpm
	mv urbi-dev-$(URBI_VERSION_SHA1)-2.$(DEB_ARCH).rpm urbi-dev-$(URBI_VERSION_SHA1)_$(DEB_ARCH).rpm

.PHONY: debian/changelog
debian/changelog: $(srcdir)/debian/changelog.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]VERSION@/$(URBI_VERSION_SHA1)/' $< > $@.tmp
	$(AM_V_at)mv $@.tmp $@

.PHONY: debian/rules
debian/rules: $(srcdir)/debian/rules.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]PACKAGE_NAME@/$(PACKAGE_NAME)/' $< > $@.tmp
	$(AM_V_at)mv $@.tmp $@

# FIXME: Factorisation with %. Gmake doesn't like %. with .PHONY.
.PHONY: debian/urbi-doc.install
debian/urbi-doc.install: $(srcdir)/debian/urbi-doc.install.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]PREFIX@/$(PREFIX)/' $< > $@.tmp
	$(AM_V_at)mv $@.tmp $@

.PHONY: debian/urbi-dev.install
debian/urbi-dev.install: $(srcdir)/debian/urbi-dev.install.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]PREFIX@/$(PREFIX)/' $< > $@.tmp
	$(AM_V_at)mv $@.tmp $@

.PHONY: debian/urbi.install
debian/urbi.install: $(srcdir)/debian/urbi.install.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]PREFIX@/$(PREFIX)/' $< > $@.tmp
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
