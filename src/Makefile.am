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
AUTOMAKE_OPTIONS += -Wno-portability

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
flavorable.hh							\
memorymanager/memorymanager.cc					\
tag-info.cc							\
uast.hh								\
uasynccommand.hh uasynccommand.cc				\
uasyncregister.cc						\
uatcandidate.hh uatcandidate.cc					\
ubanner.hh ubanner.cc						\
ubinary.hh ubinary.cc						\
ubinder.hh ubinder.hxx ubinder.cc				\
ucallid.hh ucallid.cc						\
ucommand.hh ucommand.hxx ucommand.cc				\
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
unamedparameters.hh unamedparameters.hxx unamedparameters.cc	\
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
uwait-counter.hh						\
usystem.cc

## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/libport/build-aux/revision.mk



## ---------- ##
## uobject/.  ##
## ---------- ##

include $(top_srcdir)/libport/build-aux/svn-externals.mk

include $(top_srcdir)/urbi-sdk-remote/src/liburbi/uobject/uobject.mk
nodist_libkernel_la_SOURCES += $(uobject_sources)


## ----------------- ##
## Sub directories.  ##
## ----------------- ##

include parser/parser.mk
nodist_libkernel_la_SOURCES += $(libport_sources)

urbiinclude_HEADERS = $(uobject_headers)


## -------------- ##
## urbi-console.  ##
## -------------- ##

if !OPENR
bin_PROGRAMS = urbi-console
urbi_console_SOURCES = urbi-console.cc
urbi_console_LDADD = libkernel.la

dist_libkernel_la_SOURCES +=				\
console-server.cc					\
$(top_srcdir)/libport/network/bsdnet/connection.hh	\
$(top_srcdir)/libport/network/bsdnet/connection.cc	\
$(top_srcdir)/libport/network/bsdnet/network.hh		\
$(top_srcdir)/libport/network/bsdnet/network.cc

# FIXME: We might need this guy somewhere: $(PTHREAD_LIBS).  In
# libkernel, or urbi_console_LDADD?

endif
