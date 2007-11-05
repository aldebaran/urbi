################################################################################
#
# URBI - (C) 2004, 2005, 2006 Jean-Christophe Baillie.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
################################################################################

include $(top_srcdir)/build-aux/build-aux.mk

ACLOCAL_AMFLAGS = -I build-aux

SUBDIRS = libport src tests . doc

# build-aux.
EXTRA_DIST +=					\
build-aux/bison++.in				\
build-aux/move-if-change

# windows.
EXTRA_DIST +=					\
windows/urbikernel/Debug			\
windows/urbikernel/urbikernel.sln		\
windows/urbikernel/urbikernel.suo          	\
windows/urbikernel/urbikernel.vcproj

dist-hook:
	find '$(top_distdir)' -type d -name .svn | xargs rm -rf


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
include/kernel/ustring.hh			\
include/kernel/utypes.hh			\
include/kernel/uvalue.hh 			\
include/kernel/uvariable.hh

nodist_kernelincludekernel_HEADERS =		\
include/kernel/version.hh

## --------------- ##
## Svn externals.  ##
## --------------- ##
SVN_EXTERNALS += libport tests src/uobject

maintainer-check: valgrind-check
valgrind-check:
	$(MAKE) $(AM_MAKEFLAGS) check USE_VALGRIND=1

check-html:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) $@
