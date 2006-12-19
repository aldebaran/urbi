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

include $(top_srcdir)/build-aux/init.mk

kernelinclude_HEADERS =

AM_CPPFLAGS += -I. -I$(srcdir) $(LIBPORT_CPPFLAGS) -I$(uobject_srcdir)
AM_CXXFLAGS += $(WARNING_CXXFLAGS) $(PTHREAD_CFLAGS)


## ----------- ##
## libkernel.  ##
## ----------- ##

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
memorymanager/blockmemorymanager.hh				\
memorymanager/memorymanager.hh memorymanager/memorymanager.cc	\
parser/uparser.hh parser/uparser.cc				\
uasynccommand.hh uasynccommand.cc				\
uasyncregister.hh uasyncregister.cc				\
uatcandidate.hh uatcandidate.cc					\
ubanner.hh ubanner.cc						\
ubinary.hh ubinary.cc						\
ubinder.hh ubinder.cc						\
ucallid.hh ucallid.cc						\
ucommand.hh ucommand.cc						\
ucommandqueue.hh ucommandqueue.cc				\
ucomplaints.hh ucomplaints.cc					\
uconnection.hh uconnection.cc					\
ucopy.hh							\
ueventcompound.hh ueventcompound.cc				\
ueventhandler.hh ueventhandler.cc				\
ueventinstance.hh ueventinstance.cc				\
ueventmatch.hh ueventmatch.cc					\
uexpression.hh uexpression.cc					\
ufunction.hh ufunction.cc					\
ughostconnection.hh ughostconnection.cc				\
ugroup.hh ugroup.cc						\
unamedparameters.hh unamedparameters.cc				\
uobj.hh uobj.cc							\
uobject.cc							\
uproperty.hh uproperty.cc					\
uqueue.hh uqueue.cc						\
userver.hh userver.cc						\
ustring.hh ustring.cc						\
utypes.hh							\
uvalue.hh uvalue.cc						\
uvar.cc								\
uvariable.hh uvariable.cc					\
uvariablelist.hh uvariablelist.cc				\
uvariablename.hh uvariablename.cc                               \
usystem.cc

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



## ---------- ##
## uobject/.  ##
## ---------- ##

include $(top_srcdir)/build-aux/svn-externals.mk
include uobject/uobject.mk
nodist_libkernel_la_SOURCES += $(uobject_sources)


## ----------------- ##
## Sub directories.  ##
## ----------------- ##

# parser.
EXTRA_DIST +=					\
parser/uparser.hh

# parser/bison.
# This case is handled by parser/bison/bison.mk itself.


## ----------------- ##
## Install headers.  ##
## ----------------- ##

# This is a kludge: for the time being, install all the server include
# files for the cores.
kernelinclude_HEADERS +=			\
fwd.hh						\
version.hh					\
ubinary.hh					\
ubinder.hh					\
ucallid.hh					\
ucommand.hh					\
ucommandqueue.hh				\
ucomplaints.hh					\
uconnection.hh					\
ucopy.hh					\
ueventhandler.hh				\
uexpression.hh					\
ufunction.hh					\
ughostconnection.hh				\
ugroup.hh					\
unamedparameters.hh				\
uobj.hh						\
uproperty.hh					\
uqueue.hh					\
userver.hh					\
ustring.hh					\
utypes.hh					\
uvalue.hh					\
uvariable.hh					\
uvariablelist.hh				\
uvariablename.hh				\
uasynccommand.hh				\
uasyncregister.hh				\
ueventmatch.hh					\
ueventcompound.hh				\
ueventinstance.hh				\
uatcandidate.hh

urbiinclude_HEADERS = $(uobject_headers)

parserdir = $(kernelincludedir)/parser
parser_HEADERS = parser/uparser.hh

memorymanagerdir = $(kernelincludedir)/memorymanager
memorymanager_HEADERS =				\
memorymanager/blockmemorymanager.hh		\
memorymanager/memorymanager.hh


## ---------- ##
## uconsole.  ##
## ---------- ##

if !OPENR
noinst_PROGRAMS = uconsole
uconsole_SOURCES = console.cc
uconsole_LDADD = libkernel.la $(PTHREAD_LIBS)
endif
