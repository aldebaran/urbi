#
# URBI - (C) 2004, 2005, 2006, 2007 Jean-Christophe Baillie.
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#

include $(top_srcdir)/build-aux/build-aux.mk

ACLOCAL_AMFLAGS = -I build-aux

SUBDIRS = libport src tests . doc

# build-aux.
EXTRA_DIST +=					\
build-aux/bison++.in				\
build-aux/move-if-change			\
build-aux/output-to-dot

dist-hook:
	find '$(top_distdir)' -type d -name .svn | xargs rm -rf


## -------------------------- ##
## Installed kernel headers.  ##
## -------------------------- ##

kernelincludekerneldir = $(kernelincludedir)/kernel
kernelincludekernel_HEADERS =			\
include/kernel/blockmemorymanager.hh		\
include/kernel/fwd.hh				\
include/kernel/memorymanager.hh			\
include/kernel/ucomplaints.hh			\
include/kernel/uconnection.hh			\
include/kernel/uconnection.hxx			\
include/kernel/userver.hh			\
include/kernel/userver.hxx			\
include/kernel/ustring.hh			\
include/kernel/utypes.hh

dist_pkgdata_DATA = share/urbi/urbi.u


nodist_kernelincludekernel_HEADERS =		\
include/kernel/version.hh

## --------------- ##
## Svn externals.  ##
## --------------- ##
SVN_EXTERNALS += libport tests src/uobject

maintainer-check: valgrind-check

.PHONY: valgrind-check check-html doc
valgrind-check:
	$(MAKE) $(AM_MAKEFLAGS) check USE_VALGRIND=1

check-html k2-check:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) $@

doc:
	cd doc && $(MAKE) $(AM_MAKEFLAGS) $@
