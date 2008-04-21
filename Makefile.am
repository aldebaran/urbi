################################################################################
#
# URBI - (C) 2004, 2005, 2006, 2007 Jean-Christophe Baillie.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
################################################################################
include $(top_srcdir)/build-aux/build-aux.mk

ACLOCAL_AMFLAGS = -I build-aux -I sdk

SUBDIRS = lib/libport src sdk tests . doc

# build-aux.
EXTRA_DIST +=					\
build-aux/bison++.in				\
build-aux/move-if-change			\
build-aux/output-to-dot

# dev.
devdir = dev/
include dev/dev.mk

## -------------------------- ##
## Installed kernel headers.  ##
## -------------------------- ##

kernelincludekerneldir = $(kernelincludedir)/kernel
kernelincludekernel_HEADERS =			\
include/kernel/blockmemorymanager.hh		\
include/kernel/exception.hh			\
include/kernel/exception.hxx			\
include/kernel/fwd.hh				\
include/kernel/kernconf.hh			\
include/kernel/memorymanager.hh			\
include/kernel/ucomplaints.hh			\
include/kernel/uconnection.hh			\
include/kernel/uconnection.hxx			\
include/kernel/userver.hh			\
include/kernel/userver.hxx			\
include/kernel/ustring.hh			\
include/kernel/utypes.hh

include share/urbi/local.mk

nodist_kernelincludekernel_HEADERS =		\
include/kernel/version.hh

include_libport = include/libport
include include/libport/libport.mk


## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

.PHONY: symbols
symbols:
	$(MAKE) $(AM_MAKEFLAGS) -C src $@

## --------------- ##
## Svn externals.  ##
## --------------- ##
SVN_EXTERNALS += libport tests src/uobject src/network sdk


## ------- ##
## Check.  ##
## ------- ##

# check-html, check-clean, check-uob etc.
check-%:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) $@

# The target run by the buildfarm.
CHECK_BUILDFARM_FLAGS = -j4 AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENT=1
check-buildfarm:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) check-html $(CHECK_BUILDFARM_FLAGS)
