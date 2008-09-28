include $(top_srcdir)/build-aux/build-aux.mk

ACLOCAL_AMFLAGS = -I build-aux -I sdk

# Process "." before the sub dirs to make sure sdk/config.h and
# libport/config.h are propery built.
SUBDIRS = sdk-remote . src tests doc

# build-aux.
EXTRA_DIST +=					\
  build-aux/bison++.in				\
  build-aux/flex++.in				\
  build-aux/fuse-switch				\
  build-aux/move-if-change			\
  build-aux/output-to-dot

# dev.
devdir = dev/
include dev/local.mk

include include/local.mk
include share/urbi/local.mk

## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

.PHONY: symbols
symbols:
	$(MAKE) $(AM_MAKEFLAGS) -C src $@

# check-html, check-clean, check-uob etc.
check-%:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) $@

# The target run by the buildfarm.
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENT=1
check-buildfarm:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) check-html $(CHECK_BUILDFARM_FLAGS)
