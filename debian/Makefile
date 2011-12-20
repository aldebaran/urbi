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
all:
	make -C ../_build/ dist
	mv ../_build/urbi-sdk-2.7.4-* /tmp
	tar xvjf /tmp/urbi-sdk-2.7.4-* -C /tmp
	mv /tmp/urbi-sdk-2.7.4-*.tar.bz2 /tmp/urbi-2.7.4.tar.bz2
	mv /tmp/urbi-sdk-* /tmp/urbi-2.7.4
	# FIXME: Use echo to disable interactive mode with dh_make
	# quick and dirty ...
	cd /tmp/urbi-2.7.4 && echo "hack" | dh_make -m -f ../urbi-2.7.4.tar.bz2
	rm -rf /tmp/urbi-2.7.4/debian && cp -r ../debian /tmp/urbi-2.7.4/debian
	cd /tmp/urbi-2.7.4 && dpkg-buildpackage -j4
	mv /tmp/urbi_2.7.4_i386.deb ../_build/urbi_2.7.4.deb
	mv /tmp/urbi-dev_2.7.4_i386.deb ../_build/urbi-dev_2.7.4.deb
	mv /tmp/urbi-doc_2.7.4_i386.deb ../_build/urbi-doc_2.7.4.deb
	rm -rf /tmp/urbi*


