## Copyright (C) 2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

deb:
	$(MAKE) distdir
	tardir=$(distdir) && $(am__tar) | bzip2 -9 -c >$(distdir).tar.bz2
	cp $(distdir).tar.bz2 $(PACKAGE)_$(VERSION).orig.tar.bz2
	cd $(distdir) && dpkg-buildpackage -j2

EXTRA_DIST +=					\
  debian/README.Debian				\
  debian/README.source				\
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
