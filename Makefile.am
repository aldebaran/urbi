## Copyright (C) 2006-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## Indent so that Automake does not think this endif is its.
 ifneq ($(MAKE_VERSION),3.82)
  ifeq ($(MAKELEVEL),0)
   $(warning ****************************************************)
   $(warning *** GNU Make 3.81 is known not to work properly. ***)
   $(warning *** GNU Make 3.82 is recommended.                ***)
   $(warning *** This is GNU Make $(MAKE_VERSION).                       ***)
   $(warning ****************************************************)
  endif
 endif

include build-aux/make/init.mk
include build-aux/make/package-version.mk
include build-aux/make/build-aux.mk
include build-aux/make/build-farm.mk
BOOTSTRAP_CFG = .bootstrap.cfg
include build-aux/make/bootstrap.mk

ACLOCAL_AMFLAGS = -I build-aux/m4 -I build-aux -I sdk-remote/sdk -I tests/m4

VERSIONIFYFLAGS +=					\
  --prefix=URBI_SDK_					\
  --header=sdk-remote/include/urbi/revision.hh

# To keep the time stamps up to date, we need to compile our
# dependencies first, so process sdk-remote and src before "." (where
# timestamps are).  It it tests and doc that depend on these time
# stamps, so handle them after ".".
SUBDIRS = sdk-remote src modules . tests doc

DIST_SUBDIRS = $(SUBDIRS) bison

# build-aux.
EXTRA_DIST +=					\
  build-aux/bin/bison++.in			\
  build-aux/bin/bootstrap-modules		\
  build-aux/bin/flex++.in			\
  build-aux/bin/fuse-switch			\
  build-aux/bin/output-to-dot			\
  build-aux/bin/studentize			\
  build-aux/bin/tex4ht-post

include debian/local.mk
devdir = dev/
include dev/local.mk
include include/local.mk
include share/local.mk


# Make sure config.h.in is up to date.  This looks like an Automake
# bug to me.
BUILT_SOURCES +=				\
  $(top_srcdir)/src/kernel/config.h.in		\
  src/kernel/config.h


## ------------- ##
## install-doc.  ##
## ------------- ##

doc_host = downloads@www.gostai.com
doc_upload_dir = gostai.com-downloads
doc_dir = $(doc_upload_dir)/urbi/$(PACKAGE_VERSION)/doc
install-doc/%:
	$(MAKE) $(AM_MAKEFLAGS)			\
	  doc_host='$(doc_host)'		\
	  doc_dir='$(doc_dir).new'		\
	  -C $* install-doc
## Doxygen creates *.md5 and *.map files that serve only for
## incremental builds.  Do not propagate them, they are not needed to
## browse the documentation
## (http://osdir.com/ml/text.doxygen.general/2005-04/msg00064.html).
RSYNCFLAGS =					\
  --verbose --checksum --recursive --compress	\
  --exclude '*.md5' --exclude '*.map'
install-doc:
	ssh $(doc_host)					\
	 'rm -rf $(doc_dir).new $(doc_dir).old &&	\
	  if test -d $(doc_dir); then			\
	    cp -r $(doc_dir) $(doc_dir).new;		\
	  else						\
	    mkdir -p $(doc_dir).new;			\
	  fi'
	$(MAKE) $(AM_MAKEFLAGS) RSYNCFLAGS='$(RSYNCFLAGS)'	\
	  install-doc/doc install-doc/sdk-remote
	ssh $(doc_host)							\
	  '{ test ! -e $(doc_dir) || mv $(doc_dir) $(doc_dir).old; } &&	\
	   mv $(doc_dir).new $(doc_dir) &&				\
	   rm -rf $(doc_dir).old'
## FIXME: Race condition on this script.  It would be better to
## have the script run ssh rather that the converse.
	scp $(build_aux_dir)/bin/symlink-to-latest $(doc_host):/tmp
	ssh $(doc_host) 'perl -f /tmp/symlink-to-latest $(dir $(doc_dir))'
	scp $(top_srcdir)/scripts/www/HEADER.html $(doc_host):$(dir $(doc_dir))

## ------- ##
## Check.  ##
## ------- ##

# Run just the doc test suite, without building again the PDF.
check-addons check-doc:
	$(MAKE) $(AM_MAKEFLAGS) -C doc $@

# Check the kernel: doc and tests.
check-kernel:
	$(MAKE) $(AM_MAKEFLAGS) check-doc
	$(MAKE) $(AM_MAKEFLAGS) -C tests check

# check-html, check-clean, check-uob etc.
# Last in order to avoid hiding the previous rules.
check-%:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src/tests $@
	$(MAKE) $(AM_MAKEFLAGS) -C tests $@
	$(MAKE) $(AM_MAKEFLAGS) -C doc $@
recheck recheck-html:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src/tests $@
	$(MAKE) $(AM_MAKEFLAGS) -C tests $@
	$(MAKE) $(AM_MAKEFLAGS) -C doc $@

.PHONY: spell-check
spell-check:
	$(MAKE) $(AM_MAKEFLAGS) -C doc $@

## -------- ##
## Stamps.  ##
## -------- ##

# urbi.stamp is defined and built in share/urbi/local.mk.
STAMPS += urbi.stamp

# All the libraries.
STAMPS += libraries.stamp
libraries.stamp: src/libuobject$(LIBSFX).la sdk-remote/src/libuobject/libuobject$(LIBSFX).la sdk-remote/src/liburbi/liburbi$(LIBSFX).la

# All the executables.
STAMPS += executables.stamp
EXECUTABLES = src/bin/urbi$(EXEEXT)
if !STATIC_BUILD
EXECUTABLES += sdk-remote/src/bin/urbi-launch$(EXEEXT)
endif !STATIC_BUILD
executables.stamp: $(EXECUTABLES)

# Everything.
STAMPS += all.stamp
all.stamp: urbi.stamp libraries.stamp executables.stamp


## --------- ##
## Install.  ##
## --------- ##
.PHONY: relocatable
# The buildfarm calls bf-post-install without using the relocatable
# target.  So be sure to keep these up to date all the time.
BUILT_SOURCES += 				\
  scripts/bf-post-install			\
  scripts/common.sh				\
  scripts/install-boost				\
  scripts/install-java
relocatable: scripts/bf-post-install
	$<

# Used by make-relocatable.
EXTRA_DIST +=					\
  build-aux/bin/check-lib-deps			\
  build-aux/bin/check-include-deps

if WIN32
  topleveldir = $(prefix)
  toplevel_SCRIPTS = urbi.bat
endif
