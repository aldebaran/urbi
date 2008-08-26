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


## ----------- ##
## libkernel.  ##
## ----------- ##

env_LTLIBRARIES = libkernel.la
libkernel_la_LDFLAGS = $(BOOST_THREAD_LDFLAGS)

# The separation in two different targets is needed to be sure that
# the libs are installed before we try to move them.
# install-exec-local:

#install-data-hook:
#	rm $(DESTDIR)$(envdir)/.libs
#	$(LN_S) . $(DESTDIR)$(envdir)/.libs
#	$(mkdir_p) $(DESTDIR)$(envdir)/.libs
#	mv $(DESTDIR)$(envdir)/lib* $(DESTDIR)$(envdir)/.libs
#	$(LN_S) .libs/libkernel.la $(DESTDIR)$(envdir)
#	$(LN_S) .libs/libkernel.a $(DESTDIR)$(envdir)

#uninstall-local:
#	rm -rf $(DESTDIR)$(envdir)/.libs

## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/build-aux/revision.mk


kernelinclude_HEADERS =
dist_libkernel_la_SOURCES =
nodist_libkernel_la_SOURCES =
BUILT_SOURCES += $(nodist_libkernel_la_SOURCES)
# Needed in object/local.mk, used potentially everywhere else (maybe
# before, so initialize it here).
precompiled_symbols_hh_deps =

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

include ast/local.mk
include binder/local.mk
include flower/local.mk
include parser/local.mk
include object/local.mk
include rewrite/local.mk
include runner/local.mk
include scheduler/local.mk
include scheduler/libcoroutine/local.mk
include kernel/local.mk

urbiinclude_HEADERS = $(uobject_headers)

## -------------- ##
## urbi-console.  ##
## -------------- ##

if !OPENR
bin_PROGRAMS = urbi-console ast-dump
urbi_console_SOURCES = urbi-console.cc
urbi_console_LDADD = libkernel.la $(PTHREAD_LIBS) $(LTDL_LIBS)
ast_dump_SOURCES = ast-dump.cc ast/dot-printer.cc
ast_dump_LDADD = libkernel.la $(PTHREAD_LIBS) $(LTDL_LIBS) \
	$(BOOST_REGEX_LDFLAGS) $(BOOST_REGEX_LIBS)

dist_libkernel_la_SOURCES +=			\
console-server.cc				\
../lib/network/bsdnet/connection.hh		\
../lib/network/bsdnet/connection.cc		\
../lib/network/bsdnet/network.hh		\
../lib/network/bsdnet/network.cc

libkernel_la_LIBADD = $(top_builddir)/lib/libport/libport.la

endif

