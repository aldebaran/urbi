include $(top_srcdir)/build-aux/build-aux.mk
AUTOMAKE_OPTIONS += subdir-objects
ACLOCAL_AMFLAGS = -I build-aux -I sdk

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status $@
include libltdl/Makefile.inc

SUBDIRS =					\
  .						\
  jpeg-6b lib/libport				\
  src						\
  doc

host_SCRIPTS =
nodist_host_DATA =
include sdk/local.mk
include include/local.mk


## ---------------------- ##
## Convenient shortcuts.  ##
## ---------------------- ##

.PHONY: libs
libs:
	$(MAKE) $(AM_MAKEFLAGS) -C lib/libport
	$(MAKE) -C src libs


## ------- ##
## Check.  ##
## ------- ##

# check-html, check-clean, check-uob etc.
check-%:
	$(MAKE) $(AM_MAKEFLAGS) -C src/tests $@
recheck recheck-html:
	$(MAKE) $(AM_MAKEFLAGS) -C src/tests $@

# The target run by the buildfarm.
CHECK_BUILDFARM_FLAGS = -j4 AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENT=1
check-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) -C src/tests \
	  $(CHECK_BUILDFARM_FLAGS) check-html

## ----- ##
## Doc.  ##
## ----- ##

.PHONY: doc html

doc html:
	$(MAKE) $(AM_MAKEFLAGS) -C doc $@
