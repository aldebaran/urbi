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

AM_CPPFLAGS = -I$(srcdir)/uobject
#AM_CPPFLAGS += -DENABLE_BLOCKMEMMNGR -DFLOAT_DOUBLE
AM_CPPFLAGS += -DFLOAT_DOUBLE
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

env_LTLIBRARIES = libkernel.la
dist_libkernel_la_SOURCES =					\
lockable.h							\
memorymanager/blockmemorymanager.h				\
memorymanager/memorymanager.h memorymanager/memorymanager.cc	\
ubanner.hh ubanner.cc						\
ubinary.h ubinary.cc						\
ubinder.h ubinder.cc						\
ucallid.h ucallid.cc						\
ucommand.h ucommand.cc						\
ucommandqueue.h ucommandqueue.cc				\
uconnection.h uconnection.cc					\
uexpression.h uexpression.cc					\
ufloat.h ufloat.cc						\
ufunction.h ufunction.cc					\
ughostconnection.h ughostconnection.cc				\
ugroup.h ugroup.cc						\
unamedparameters.h unamedparameters.cc				\
uobj.h uobj.cc							\
uobject/common_uvalue.cc					\
uobject/uext.h							\
uobject/uobject.h uobject/uobject.cc				\
uobject/uvar.cc							\
uproperty.h uproperty.cc					\
uqueue.h uqueue.cc						\
userver.h userver.cc						\
ustring.h ustring.cc						\
utypes.h							\
uvalue.h uvalue.cc						\
uvariable.h uvariable.cc					\
uvariablelist.h uvariablelist.cc				\
uvariablename.h uvariablename.cc


# We build for aibo, or posix.
if NETWORK_OPENR
include network/OPENR/openr.mk
else
include network/bsdnet/bsdnet.mk
endif

include parser/bison/bison.mk

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

# parser.
EXTRA_DIST +=					\
parser/uparser.h

# parser/bison.
# This case is handled by parser/bison/Makefile.defs itself.

# windows.
EXTRA_DIST +=					\
windows/urbikernel/Debug			\
windows/urbikernel/urbikernel.sln		\
windows/urbikernel/urbikernel.suo          	\
windows/urbikernel/urbikernel.vcproj       


## ----------------- ##
## Install headers.  ##
## ----------------- ##

# This is a kludge: for the time being, install all the server include
# files for the cores.
kernelinclude_HEADERS =				\
lockable.h					\
ubinary.h					\
ubinder.h					\
ucallid.h					\
ucommand.h					\
ucommandqueue.h					\
uconnection.h					\
uexpression.h					\
ufloat.h					\
ufunction.h					\
ughostconnection.h				\
ugroup.h					\
unamedparameters.h				\
uobj.h 						\
uproperty.h					\
uqueue.h					\
userver.h					\
ustring.h					\
utypes.h					\
uvalue.h					\
uvariable.h					\
uvariablelist.h					\
uvariablename.h 

uobjectdir = $(kernelincludedir)/uobject
uobject_HEADERS = uobject/uobject.h

parserdir = $(kernelincludedir)/parser
parser_HEADERS = parser/uparser.h

memorymanagerdir = $(kernelincludedir)/memorymanager
memorymanager_HEADERS =				\
memorymanager/blockmemorymanager.h		\
memorymanager/memorymanager.h

