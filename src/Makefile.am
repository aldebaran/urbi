################################################################################
#
# URBI - Copyright (C) 2004, 2005, 2006 Jean-Christophe Baillie 
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
################################################################################

## ---------------- ##
## Initialization.  ##
## ---------------- ##

BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =
kernelinclude_HEADERS =

liburbi_srcdir = $(top_srcdir)/liburbi-c++/src/liburbi
uobject_srcdir = $(liburbi_srcdir)/uobject

AM_CPPFLAGS = -I$(top_srcdir) -I$(liburbi_srcdir) -I$(uobject_srcdir)
# FIXME: What is this?  Where is it used?
AM_CPPFLAGS += -DFLOAT_DOUBLE
#AM_CPPFLAGS += -DENABLE_BLOCKMEMMNGR
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

env_LTLIBRARIES = libkernel.la
libkernel_la_LDFLAGS = -static

install-exec-local:
	sed -e 's/libdir=.*/libdir=/' \
	    -e 's/installed=.*/installed=no/' \
	    .libs/libkernel.lai >libkernel.lai.new
	mv -f libkernel.lai.new .libs/libkernel.lai

install-data-hook:
	mkdir -p $(DESTDIR)$(envdir)/.libs
	mv $(DESTDIR)$(envdir)/lib* $(DESTDIR)$(envdir)/.libs
	ln -s .libs/libkernel.la $(DESTDIR)$(envdir)
	ln -s .libs/libkernel.a $(DESTDIR)$(envdir)

uninstall-local:
	rm -rf $(DESTDIR)$(envdir)/.libs

dist_libkernel_la_SOURCES =					\
fwd.hh								\
memorymanager/blockmemorymanager.h				\
memorymanager/memorymanager.h memorymanager/memorymanager.cc	\
parser/uparser.h parser/uparser.cc				\
ubanner.hh ubanner.cc						\
ubinary.h ubinary.cc						\
ubinder.h ubinder.cc						\
ucallid.h ucallid.cc						\
ucommand.h ucommand.cc						\
ucommandqueue.h ucommandqueue.cc				\
uconnection.h uconnection.cc					\
ueventhandler.h ueventhandler.cc				\
uexpression.h uexpression.cc					\
ufunction.h ufunction.cc					\
ughostconnection.h ughostconnection.cc				\
ugroup.h ugroup.cc						\
unamedparameters.h unamedparameters.cc				\
uobj.h uobj.cc							\
$(uobject_srcdir)/common_uvalue.cc				\
$(uobject_srcdir)/uobject.h uobject.cc				\
uvar.cc								\
uproperty.h uproperty.cc					\
uqueue.h uqueue.cc						\
userver.h userver.cc						\
ustring.h ustring.cc						\
utypes.h							\
uvalue.h uvalue.cc						\
uvariable.h uvariable.cc					\
uvariablelist.h uvariablelist.cc				\
uvariablename.h uvariablename.cc

# We either build the BSD connection set, or none at all for Aibo.
if !OPENR
include network/bsdnet/bsdnet.mk
endif

include parser/bison/bison.mk

## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/build-aux/revision.mk



## ----------------- ##
## Sub directories.  ##
## ----------------- ##

# parser.
EXTRA_DIST +=					\
parser/uparser.h

# parser/bison.
# This case is handled by parser/bison/bison.mk itself.


## ----------------- ##
## Install headers.  ##
## ----------------- ##

# This is a kludge: for the time being, install all the server include
# files for the cores.
kernelinclude_HEADERS +=			\
fwd.hh						\
ubinary.h					\
ubinder.h					\
ucallid.h					\
ucommand.h					\
ucommandqueue.h					\
uconnection.h					\
ueventhandler.h					\
uexpression.h					\
ufunction.h					\
ughostconnection.h				\
ugroup.h					\
unamedparameters.h				\
uobj.h						\
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
uobject_HEADERS = $(uobject_srcdir)/uobject.h

parserdir = $(kernelincludedir)/parser
parser_HEADERS = parser/uparser.h

memorymanagerdir = $(kernelincludedir)/memorymanager
memorymanager_HEADERS =				\
memorymanager/blockmemorymanager.h		\
memorymanager/memorymanager.h


## ---------- ##
## uconsole.  ##
## ---------- ##

if !OPENR
noinst_PROGRAMS = uconsole
uconsole_SOURCES = console.cc
uconsole_LDADD = libkernel.la
endif
