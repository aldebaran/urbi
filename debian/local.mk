## Copyright (C) 2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

PACKAGE_NAME = urbi_$(PACKAGE_VERSION)

deb: debian/changelog
	$(MAKE) distdir
	mv $(distdir) $(PACKAGE_NAME)
	tardir=$(PACKAGE_NAME) && $(am__tar) | bzip2 -9 -c >$(PACKAGE_NAME).tar.bz2
	cp $(PACKAGE_NAME).tar.bz2 $(PACKAGE_NAME).orig.tar.bz2
	cd $(PACKAGE_NAME) && dpkg-buildpackage -j2

rpm: deb
	fakeroot
	sudo alien -r urbi_$(PACKAGE_VERSION)
	sudo alien -r urbi-doc_$(PACKAGE_VERSION)
	sudo alien -r urbi-dev_$(PACKAGE_VERSION)

.PHONY: debian/changelog
debian/changelog: $(srcdir)/debian/changelog.in
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)sed -e 's/[@]VERSION@/$(PACKAGE_VERSION)/' $< > $@.tmp
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
  debian/rules					\
  debian/source/format				\
  debian/urbi-dev.install			\
  debian/urbi-doc.docs				\
  debian/urbi-doc.install			\
  debian/urbi.install
