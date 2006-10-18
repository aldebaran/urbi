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

## --------- ##
## liburbi.  ##
## --------- ##

# We use some of its files, so to ensure a correct synchronization, we
# pin the revision we're using.  These targets simplify upgrading the
# pin revision.
.PHONY: liburbi-up liburbi-ci

# Update the dependency on liburbi.
liburbi-up:
	cd $(top_srcdir) && build-aux/svn-externals --update=liburbi-c++

# Checkin the sub liburbi, and update the dependency.
liburbi-ci:
	cd $(top_srcdir)/liburbi-c++ && vcs-svn ci
	$(MAKE) $(AM_MAKEFLAGS) liburbi-up


## Autoconf extensions.
ACLOCAL_AMFLAGS = -I build-aux

SUBDIRS = liburbi-c++ src doc .
