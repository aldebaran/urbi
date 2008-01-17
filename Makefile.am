################################################################################
#
# URBI - (C) 2004, 2005, 2006 Jean-Christophe Baillie.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
################################################################################

include $(top_srcdir)/libport/build-aux/build-aux.mk

ACLOCAL_AMFLAGS = -I libport/build-aux -I libport/sdk

SUBDIRS = libport/include libport/lib src libport/sdk tests . doc

# build-aux.
EXTRA_DIST +=						\
libport/build-aux/bison++.in				\
libport/build-aux/move-if-change			\
libport/build-aux/output-to-dot

# windows.
EXTRA_DIST +=					\
windows/urbikernel/Debug			\
windows/urbikernel/urbikernel.sln		\
windows/urbikernel/urbikernel.suo          	\
windows/urbikernel/urbikernel.vcproj

## -------------------------- ##
## Installed kernel headers.  ##
## -------------------------- ##

kernelincludekerneldir = $(kernelincludedir)/kernel
kernelincludekernel_HEADERS =			\
include/kernel/blockmemorymanager.hh		\
include/kernel/fwd.hh				\
include/kernel/mem-track.hh			\
include/kernel/memorymanager.hh			\
include/kernel/tag-info.hh			\
include/kernel/uasyncregister.hh 		\
include/kernel/ucomplaints.hh			\
include/kernel/uconnection.hh			\
include/kernel/userver.hh			\
include/kernel/userver.hxx			\
include/kernel/ustring.hh			\
include/kernel/utypes.hh			\
include/kernel/uvalue.hh 			\
include/kernel/uvariable.hh

nodist_kernelincludekernel_HEADERS =		\
include/kernel/version.hh

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
