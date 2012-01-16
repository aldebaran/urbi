## Copyright (C) 2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

URBI_VERSION = $(shell echo $(VERSION) | sed -e 's/-/./g')
## The root of the Debian packages.
PACKAGE_DEBIAN = urbi_$(URBI_VERSION)
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
	$(AM_V_GEN)$(MAKE) distdir
	$(AM_V_at)mkdir -p debian
	$(AM_V_at)for i in $(DEBIAN_GENERATED);			\
	do							\
	  sed							\
	    -e 's/[@]VERSION@/$(URBI_VERSION)/'			\
	    -e 's/[@]PACKAGE_DEBIAN@/$(PACKAGE_DEBIAN)/'	\
	    -e 's,[@]PREFIX@,$(prefix),'			\
	    $(srcdir)/$$i.in > $$i.tmp &&			\
	  mv $$i.tmp $(distdir)/$$i;				\
	done
	rm -rf $(PACKAGE_DEBIAN)
	mv $(distdir) $(PACKAGE_DEBIAN)
	tardir=$(PACKAGE_DEBIAN) && $(am__tar) | bzip2 -9 -c >$(PACKAGE_DEBIAN).tar.bz2
	cp $(PACKAGE_DEBIAN).tar.bz2 $(PACKAGE_DEBIAN).orig.tar.bz2
	cd $(PACKAGE_DEBIAN) && dpkg-buildpackage

rpm:
	fakeroot -- alien -r urbi_$(URBI_VERSION)_$(DEB_ARCH).deb
	mv urbi-$(URBI_VERSION)-2.$(DEB_ARCH).rpm urbi-$(URBI_VERSION)_$(DEB_ARCH).rpm
	fakeroot -- alien -r urbi-doc_$(URBI_VERSION)_$(DEB_ARCH).deb
	mv urbi-doc-$(URBI_VERSION)-2.$(DEB_ARCH).rpm urbi-doc-$(URBI_VERSION)_$(DEB_ARCH).rpm
	fakeroot -- alien -r urbi-dev_$(URBI_VERSION)_$(DEB_ARCH).deb
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
  debian/source/format				\
  debian/urbi-doc.docs
