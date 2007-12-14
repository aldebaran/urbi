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
AUTOMAKE_OPTIONS += -Wno-portability

kernelinclude_HEADERS =
dist_libkernel_la_SOURCES =
nodist_libkernel_la_SOURCES =

## ----------- ##
## libkernel.  ##
## ----------- ##

env_LTLIBRARIES = libkernel.la
libkernel_la_LDFLAGS = -static $(BOOST_THREAD_LDFLAGS)

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

dist_libkernel_la_SOURCES +=					\
flavorable.hh							\
memorymanager/memorymanager.cc					\
uast.hh								\
ubanner.hh ubanner.cc						\
ubinary.hh ubinary.cc						\
ubinder.hh ubinder.hxx ubinder.cc				\
ucallid.hh ucallid.cc						\
ucommand.hh ucommand.hxx ucommand.cc				\
ucommandqueue.hh ucommandqueue.cc				\
ucomplaints.cc							\
uconnection.cc							\
ughostconnection.hh ughostconnection.cc				\
uqueue.hh uqueue.cc						\
userver.cc							\
ustring.cc							\
uwait-counter.hh						\
usystem.cc


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
# nodist_libkernel_la_SOURCES += $(uobject_sources)


## ----------------- ##
## Sub directories.  ##
## ----------------- ##

include ast/ast.mk
include parser/parser.mk
include object/object.mk
include runner/runner.mk


urbiinclude_HEADERS = $(uobject_headers)


## -------------- ##
## urbi-console.  ##
## -------------- ##

if !OPENR
noinst_PROGRAMS = urbi-console
uconsole_SOURCES =				\
console-server.cc				\
network/bsdnet/connection.hh			\
network/bsdnet/connection.cc			\
network/bsdnet/network.hh			\
network/bsdnet/network.cc
uconsole_LDADD = libkernel.la $(top_builddir)/libport/libport.la
endif
