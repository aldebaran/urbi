################################################################################
#
# URBI - Copyright (C) 2004, 2005, 2006, 2007, 2008 Jean-Christophe Baillie
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
AUTOMAKE_OPTIONS += -Wno-portability subdir-objects

kernelinclude_HEADERS =
dist_libkernel_la_SOURCES =
nodist_libkernel_la_SOURCES =



## ----------- ##
## libkernel.  ##
## ----------- ##

env_LTLIBRARIES = libkernel.la
libkernel_la_LDFLAGS = -static $(BOOST_THREAD_LDFLAGS)

# The separation in two different targets is needed to be sure that
# the libs are installed before we try to move them.
install-exec-local:
	sed -e 's/libdir=.*/libdir=/' \
	    -e 's/installed=.*/installed=no/' \
	    .libs/libkernel.lai >libkernel.lai.new
	mv -f libkernel.lai.new .libs/libkernel.lai

install-data-hook:
	$(mkdir_p) $(DESTDIR)$(envdir)/.libs
	mv $(DESTDIR)$(envdir)/lib* $(DESTDIR)$(envdir)/.libs
	$(LN_S) .libs/libkernel.la $(DESTDIR)$(envdir)
	$(LN_S) .libs/libkernel.a $(DESTDIR)$(envdir)

uninstall-local:
	rm -rf $(DESTDIR)$(envdir)/.libs

dist_libkernel_la_SOURCES +=					\
memorymanager/memorymanager.cc					\
server-timer.hh server-timer.cc					\
ubanner.hh ubanner.cc						\
ucommandqueue.hh ucommandqueue.cc				\
ucomplaints.cc							\
uconnection.cc							\
ughostconnection.hh ughostconnection.cc				\
uobject.cc                                                      \
uobject.hh                                                      \
uqueue.hh uqueue.cc						\
userver.cc							\
ustring.cc							\
usystem.cc                                                      \
uvalue-cast.cc							\
uvalue-cast.hh


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
dist_libkernel_la_SOURCES +=				\
$(dist_uobject_sources)



## ----------------- ##
## Sub directories.  ##
## ----------------- ##

include ast/ast.mk
include parser/parser.mk
include object/object.mk
include runner/runner.mk
include scheduler/scheduler.mk
include scheduler/libcoroutine/libcoroutine.mk

urbiinclude_HEADERS = $(uobject_headers)


## -------------- ##
## urbi-console.  ##
## -------------- ##

if !OPENR
noinst_PROGRAMS = urbi-console
urbi_console_SOURCES = urbi-console.cc
urbi_console_LDADD = libkernel.la $(PTHREAD_LIBS)

dist_libkernel_la_SOURCES +=			\
console-server.cc				\
../lib/network/bsdnet/connection.hh		\
../lib/network/bsdnet/connection.cc		\
../lib/network/bsdnet/network.hh		\
../lib/network/bsdnet/network.cc

libkernel_la_LIBADD = $(top_builddir)/lib/libport/libport.la

# FIXME: We might need these guys somewhere: $(PTHREAD_LIBS).  In
# libkernel, or urbi_console_LDADD?
# urbi_console_LDADD = libkernel.la libport.la $(PTHREAD_LIBS)

endif
