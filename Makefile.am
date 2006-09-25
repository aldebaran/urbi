################################################################################
#
# URBI - (C) Jean-Christophe Baillie, 2004-2005 
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
################################################################################

# Initialization.
EXTRA_DIST =
BUILT_SOURCES =

## Autoconf extensions.
ACLOCAL_AMFLAGS = -I build-aux

SUBDIRS = . doc

AM_CPPFLAGS = -Inetwork/$(NETWORK) -I. -I$(srcdir)
#AM_CPPFLAGS += -DENABLE_BLOCKMEMMNGR -DFLOAT_DOUBLE
AM_CPPFLAGS += -DFLOAT_DOUBLE
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

lib_LTLIBRARIES = libkernel.la
dist_libkernel_la_SOURCES =			\
lockable.h					\
memorymanager/memorymanager.cc			\
ubinary.h ubinary.cc				\
ubinder.h ubinder.cc				\
ucallid.h ucallid.cc				\
ucommand.h ucommand.cc				\
ucommandqueue.h ucommandqueue.cc		\
uconnection.h uconnection.cc			\
uexpression.h uexpression.cc			\
ufloat.h ufloat.cc				\
ufunction.h ufunction.cc			\
ughostconnection.h ughostconnection.cc		\
ugroup.h ugroup.cc				\
unamedparameters.h unamedparameters.cc		\
uobj.h uobj.cc					\
uobject/common_uvalue.cc			\
uobject/uobject.h uobject/uobject.cc		\
uobject/uvar.cc					\
uproperty.h uproperty.cc			\
uqueue.h uqueue.cc				\
userver.h userver.cc				\
ustring.h ustring.cc				\
utypes.h					\
uvalue.h uvalue.cc				\
uvariable.h uvariable.cc			\
uvariablelist.h uvariablelist.cc		\
uvariablename.h uvariablename.cc


if NETWORK_OPENR
include network/OPENR/Makefile.defs
endif

if NETWORK_BSDNET
include network/bsdnet/Makefile.defs
endif NETWORK_BSDNET

include parser/bison/Makefile.defs

## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include build-aux/revision.mk



## ----------------- ##
## Sub directories.  ##
## ----------------- ##

# build-aux.
EXTRA_DIST += \
build-aux/bison++.in				\
build-aux/move-if-change

# memorymanager.
EXTRA_DIST +=					\
memorymanager/blockmemorymanager.h		\
memorymanager/memorymanager.h	   		\
memorymanager/memorymanager.cc

# parser.
EXTRA_DIST +=					\
parser/uparser.h

# parser/bison.
# This case is handled by parser/bison/Makefile.defs itself.

# uobject.
EXTRA_DIST +=					\
uobject/common_uvalue.cc			\
uobject/uobject.h	   uobject/uobject.cc	\
uobject/uvar.cc

# windows.
EXTRA_DIST +=					\
windows/urbikernel/Debug			\
windows/urbikernel/urbikernel.sln		\
windows/urbikernel/urbikernel.suo          	\
windows/urbikernel/urbikernel.vcproj       
