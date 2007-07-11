################################################################################
#
# URBI - Copyright (C) 2004, 2005, 2006, 2007 Jean-Christophe Baillie
#
# This software is provided "as is" without warranty of any kind,
# either expressed or implied, including but not limited to the
# implied warranties of fitness for a particular purpose.
#
################################################################################

## ---------------- ##
## Initialization.  ##
## ---------------- ##

include kernel.mk

SUBDIRS = ast object runner .

# Defined in sub mk files, e.g., src/parser/bison/bison.mk.
# FIXME: Might not be needed after all.
kernelinclude_HEADERS =

## ----------- ##
## libkernel.  ##
## ----------- ##

env_LTLIBRARIES = libkernel.la
libkernel_la_LDFLAGS = -static
libkernel_la_LIBADD = \
ast/libast.la object/libobject.la runner/librunner.la $(libport_la)

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
flavorable.hh							\
memorymanager/memorymanager.cc					\
parser/uparser.hh parser/uparser.cc				\
tag-info.cc							\
uast.hh								\
uasynccommand.hh uasynccommand.cc				\
uasyncregister.cc						\
uatcandidate.hh uatcandidate.cc					\
ubanner.hh ubanner.cc						\
ubinary.hh ubinary.cc						\
ubinder.hh ubinder.cc						\
ucallid.hh ucallid.cc						\
ucommand.hh ucommand.cc						\
ucommandqueue.hh ucommandqueue.cc				\
ucomplaints.cc							\
uconnection.cc							\
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
userver.cc							\
ustring.cc							\
uvalue.cc							\
uvar.cc								\
uvariable.cc							\
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


urbiinclude_HEADERS = $(uobject_headers)

## parserdir = $(kernelincludedir)/parser
## parser_HEADERS = parser/uparser.hh
##
## memorymanagerdir = $(kernelincludedir)/memorymanager
## memorymanager_HEADERS =				\
## memorymanager/blockmemorymanager.hh		\
## memorymanager/memorymanager.hh


## ---------- ##
## uconsole.  ##
## ---------- ##

if !OPENR
noinst_PROGRAMS = uconsole
uconsole_SOURCES = console.cc
uconsole_LDADD = libkernel.la $(PTHREAD_LIBS)
endif

# Clean VC++ debug file. BTW, Do we want to install it?
CLEANFILES += vc80.pdb
