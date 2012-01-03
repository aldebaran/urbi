## Copyright (C) 2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

# The packages are made by:
# - doing a tarball of the kernel
# - renaming it to urbi-<version>.tar.bz2
# - extraction and renaming of the directory with the same format
# - dpkg-buildpackage -j6

# FIXME: link it to autoconf, automake to use variables and
# to be more configurable.

# FIXME: we are assuming that the build is done in a _build directory which is
# not always the case.
#
# FIXME: the version number will be linked using automake.
deb:
	$(MAKE) distdir $(distdir).tar.bz2
# FIXME: Use echo to disable interactive mode with dh_make
# quick and dirty ...
	cd $(distdir) && echo "hack" | dh_make -m -f ../$(distdir).tar.bz2
	cd $(distdir) && dpkg-buildpackage -j2
##	mv /tmp/urbi_2.7.4_i386.deb ../_build/urbi_2.7.4.deb
##	mv /tmp/urbi-dev_2.7.4_i386.deb ../_build/urbi-dev_2.7.4.deb
##	mv /tmp/urbi-doc_2.7.4_i386.deb ../_build/urbi-doc_2.7.4.deb
##	rm -rf /tmp/urbi*

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
