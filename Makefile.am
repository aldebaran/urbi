include $(top_srcdir)/build-aux/build-aux.mk

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

libs:
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/lib/libport
	$(MAKE) $(AM_MAKEFLAGS) -C sdk-remote/src libs
	$(MAKE) $(AM_MAKEFLAGS) -C src libuobject.la


## ------- ##
## Check.  ##
## ------- ##

# check-html, check-clean, check-uob etc.
check-%:
	$(MAKE) $(AM_MAKEFLAGS) -C tests $@

# The target run by the buildfarm.
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENT=1
check-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) -C tests check-html $(CHECK_BUILDFARM_FLAGS)
