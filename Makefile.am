include build-aux/init.mk
include build-aux/build-aux.mk

ACLOCAL_AMFLAGS = -I build-aux -I tests/m4

SUBDIRS = sdk-remote . src tests doc

# build-aux.
EXTRA_DIST +=					\
  build-aux/bison++.in				\
  build-aux/fix-libtool-la			\
  build-aux/flex++.in				\
  build-aux/fuse-switch				\
  build-aux/move-if-change			\
  build-aux/output-to-dot

# dev.
devdir = dev/
include dev/local.mk

include include/local.mk
include share/urbi/local.mk

# Make sure the packaging script is up-to-date.
all-local: scripts/package

## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

.PHONY: symbols
symbols:
	$(MAKE) $(AM_MAKEFLAGS) -C src $@



## ---------------------- ##
## Convenient shortcuts.  ##
## ---------------------- ##

# Don't bounce to libs: sort of loop unrolling.
core:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/lib/libport
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src core
	$(MAKE) $(AM_MAKEFLAGS) -C src libuobject.la urbi-console$(EXEEXT)

libs:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/lib/libport
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src libs
	$(MAKE) $(AM_MAKEFLAGS) -C src libuobject.la


## ------- ##
## Check.  ##
## ------- ##


# The target run by the buildfarm.
.PHONY: check-buildfarm check-sdk-uobjects-buildfarm
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENT=1
check-buildfarm:
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
