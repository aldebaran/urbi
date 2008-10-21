include $(top_srcdir)/build-aux/build-aux.mk

# That's where aclocal extensions are.
ACLOCAL_AMFLAGS = -I build-aux -I sdk

SUBDIRS =					\
  .						\
  jpeg-6b lib/libport				\
  src						\
  doc

host_SCRIPTS =
nodist_host_DATA =
include sdk/local.mk
include include/local.mk

EXTRA_DIST +=					\
  build-aux/fix-libtool-la

## ------- ##
## Check.  ##
## ------- ##

# check-html, check-clean, check-uob etc.
check-%:
	cd src/liburbi/tests && \
	  $(MAKE) $(AM_MAKEFLAGS) $@

# The target run by the buildfarm.
CHECK_BUILDFARM_FLAGS = -j4 AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENT=1
check-buildfarm:
	cd src/liburbi/tests && \
	  $(MAKE) $(AM_MAKEFLAGS) $(CHECK_BUILDFARM_FLAGS) check-html

## ----- ##
## Doc.  ##
## ----- ##

.PHONY: doc html

doc html:
	cd doc && $(MAKE) $(AM_MAKEFLAGS) $@
