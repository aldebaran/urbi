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
include $(top_srcdir)/libport/libport.mk

ACLOCAL_AMFLAGS = -I build-aux

SUBDIRS = src tests . doc

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

## --------------- ##
## Svn externals.  ##
## --------------- ##
SVN_EXTERNALS += tests src/uobject

maintainer-check: valgrind-check
valgrind-check:
	$(MAKE) $(AM_MAKEFLAGS) check USE_VALGRIND=1

check-html:
	cd tests && $(MAKE) $(AM_MAKEFLAGS) $@
