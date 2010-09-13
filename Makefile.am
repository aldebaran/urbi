## Copyright (C) 2006-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include build-aux/init.mk
include build-aux/package-version.mk
include build-aux/build-aux.mk
DISTCHECK_BUILDFARM_HOSTNAME = bf-2
include build-aux/build-farm.mk
BOOTSTRAP_CFG = .bootstrap.cfg
include build-aux/bootstrap.mk

ACLOCAL_AMFLAGS = -I build-aux -I tests/m4

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
  build-aux/bin/move-if-change			\
  build-aux/bin/output-to-dot			\
  build-aux/bin/studentize

# dev.
devdir = dev/
include dev/local.mk

include include/local.mk
include share/urbi/local.mk
include share/images/local.mk

EXTRA_DIST += urbi-sdk.key


## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

.PHONY: symbols
symbols:
	$(MAKE) $(AM_MAKEFLAGS) -C src $@


# Make sure config.h.in is up to date.  This looks like an Automake
# bug to me.
BUILT_SOURCES +=				\
  $(top_srcdir)/src/kernel/config.h.in		\
  src/kernel/config.h


## ---------------------- ##
## Convenient shortcuts.  ##
## ---------------------- ##

# Don't bounce to libs: sort of loop unrolling.
core:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/libport
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src core
	$(MAKE) $(AM_MAKEFLAGS) -C src libuobject$(LIBSFX).la bin/urbi$(EXEEXT)

libs:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/libport
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src libs
	$(MAKE) $(AM_MAKEFLAGS) -C src libuobject$(LIBSFX).la

install-doc:
	$(MAKE) $(AM_MAKEFLAGS) -C doc $@


## ------- ##
## Check.  ##
## ------- ##


# The target run by the buildfarm.
.PHONY: check-buildfarm check-sdk-uobjects-buildfarm
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 # INSTRUMENTATION=1
%check-buildfarm: clean-semaphores
	$(MAKE) $(AM_MAKEFLAGS) -C tests $*check-html $(CHECK_BUILDFARM_FLAGS)

# The target used by the build farm to create the uobjects used in our
# uob test suite.
check-sdk-uobjects-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src/tests check-uobjects $(CHECK_BUILDFARM_FLAGS)

# Run just the doc test suite, without building again the PDF.
check-doc:
	$(MAKE) $(AM_MAKEFLAGS) -C doc just-check

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
	@echo "$$(date): $?" >>$@

# All the executables.
STAMPS += executables.stamp
EXECUTABLES = src/bin/urbi$(EXEEXT)
if !STATIC_BUILD
EXECUTABLES += sdk-remote/src/bin/urbi-launch$(EXEEXT)
endif !STATIC_BUILD

executables.stamp: $(EXECUTABLES)
	@echo "$$(date): $?" >>$@

# Everything.
STAMPS += all.stamp
all.stamp: urbi.stamp libraries.stamp executables.stamp
	@echo "$$(date): $?" >>$@


## --------- ##
## Install.  ##
## --------- ##
.PHONY: relocatable
relocatable: scripts/bf-post-install scripts/install-boost scripts/package
	$<

# Used by make-relocatable.
EXTRA_DIST +=					\
  build-aux/bin/check-lib-deps			\
  build-aux/bin/check-include-deps		\
  src/find-boost.cc

if WIN32
  topleveldir = $(prefix)
  toplevel_SCRIPTS = urbi.bat
endif
