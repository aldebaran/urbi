include build-aux/init.mk
include build-aux/package-version.mk
include build-aux/build-aux.mk
include build-aux/build-farm.mk

ACLOCAL_AMFLAGS = -I build-aux -I tests/m4

# To keep the time stamps up to date, we need to compile our
# dependencies first, so process sdk-remote and src before "." (where
# timestamps are).  It it tests and doc that depend on these time
# stamps, so handle them after ".".
SUBDIRS = sdk-remote src . tests doc
DIST_SUBDIRS = $(SUBDIRS) bison

# build-aux.
EXTRA_DIST +=					\
  build-aux/bison++.in				\
  build-aux/flex++.in				\
  build-aux/fuse-switch				\
  build-aux/move-if-change			\
  build-aux/output-to-dot			\
  build-aux/studentize

# dev.
devdir = dev/
include dev/local.mk

include include/local.mk
include share/urbi/local.mk

EXTRA_DIST += urbi-sdk.key

# Make sure the packaging script is up-to-date.
all-local: scripts/package

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
	$(MAKE) $(AM_MAKEFLAGS) -C src libuobject.la bin/urbi$(EXEEXT)

libs:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/libport
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src libs
	$(MAKE) $(AM_MAKEFLAGS) -C src libuobject.la

install-doc:
	$(MAKE) $(AM_MAKEFLAGS) -C doc $@


## ------- ##
## Check.  ##
## ------- ##


# The target run by the buildfarm.
.PHONY: check-buildfarm check-sdk-uobjects-buildfarm
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 # INSTRUMENT=1
check-buildfarm:
	-$(build_aux_dir)/semaphore-clean.sh
	$(MAKE) $(AM_MAKEFLAGS) -C tests check-html $(CHECK_BUILDFARM_FLAGS)

# The target used by the build farm to create the uobjects used in our
# uob test suite.
check-sdk-uobjects-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src/tests check-uobjects $(CHECK_BUILDFARM_FLAGS)

# check-html, check-clean, check-uob etc.
# Last in order to avoid hiding the previous rules.
check-%:
	$(MAKE) $(AM_MAKEFLAGS) -C tests $@
recheck recheck-html:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src/tests $@
	$(MAKE) $(AM_MAKEFLAGS) -C tests $@


## -------- ##
## Stamps.  ##
## -------- ##

# urbi.stamp is defined and built in share/urbi/local.mk.
STAMPS += urbi.stamp

# All the libraries.
STAMPS += libraries.stamp
libraries.stamp: src/libuobject.la sdk-remote/libraries.stamp
	@echo "$$(date): $?" >>$@

# All the executables.
STAMPS += executables.stamp
executables.stamp: src/bin/urbi$(EXEEXT) sdk-remote/executables.stamp
	@echo "$$(date): $?" >>$@

# Everything.
STAMPS += all.stamp
all.stamp: urbi.stamp libraries.stamp executables.stamp
	@echo "$$(date): $?" >>$@
