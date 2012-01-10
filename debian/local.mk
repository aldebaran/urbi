## Copyright (C) 2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

URBI_VERSION = $(shell echo $(VERSION) | sed -e 's/-/./g')
DEBIAN_PACKAGE_NAME = urbi_$(URBI_VERSION)
DEB_ARCH = $(shell dpkg-architecture -qDEB_BUILD_ARCH)

.PHONY: packages deb rpm
packages: deb rpm

DEBIAN_GENERATED =				\
  debian/changelog				\
  debian/rules					\
  debian/urbi-dev.install			\
  debian/urbi-doc.install			\
  debian/urbi.install

deb:
	$(AM_V_GEN)for i in $(DEBIAN_GENERATED);			\
	do								\
	  sed								\
	    -e 's/[@]VERSION@/$(URBI_VERSION)/'				\
	    -e 's/[@]DEBIAN_PACKAGE_NAME@/$(DEBIAN_PACKAGE_NAME)/'	\
	    -e 's,[@]PREFIX@,$(prefix),'				\
	    $(srcdir)/$$i.in > $$i.tmp &&				\
	  mv $$i.tmp $$i;						\
	done
	$(MAKE) distdir
	rm -rf $(DEBIAN_PACKAGE_NAME)
	mv $(distdir) $(DEBIAN_PACKAGE_NAME)
	tardir=$(DEBIAN_PACKAGE_NAME) && $(am__tar) | bzip2 -9 -c >$(DEBIAN_PACKAGE_NAME).tar.bz2
	cp $(DEBIAN_PACKAGE_NAME).tar.bz2 $(DEBIAN_PACKAGE_NAME).orig.tar.bz2
	cd $(DEBIAN_PACKAGE_NAME) && dpkg-buildpackage

rpm:
	fakeroot -- alien -r urbi_$(URBI_VERSION)_$(DEB_ARCH).deb
	fakeroot -- alien -r urbi-doc_$(URBI_VERSION)_$(DEB_ARCH).deb
	fakeroot -- alien -r urbi-dev_$(URBI_VERSION)_$(DEB_ARCH).deb
	mv urbi-$(URBI_VERSION)-2.$(DEB_ARCH).rpm urbi-$(URBI_VERSION)_$(DEB_ARCH).rpm
	mv urbi-doc-$(URBI_VERSION)-2.$(DEB_ARCH).rpm urbi-doc-$(URBI_VERSION)_$(DEB_ARCH).rpm
	mv urbi-dev-$(URBI_VERSION)-2.$(DEB_ARCH).rpm urbi-dev-$(URBI_VERSION)_$(DEB_ARCH).rpm

EXTRA_DIST +=					\
  $(DEBIAN_GENERATED:=.in)			\
  debian/README.Debian				\
  debian/README.source				\
  debian/compat					\
  debian/control				\
  debian/copyright				\
  debian/docs					\
  debian/info					\
  debian/rules					\
  debian/source/format				\
  debian/urbi-doc.docs
