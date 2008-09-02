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
## libuobject.  ##
## ----------- ##

env_LTLIBRARIES = libuobject.la
#the -DBUILDING_LIBPORT is required since libport/network/* are considered as kernel sources.
libuobject_la_CPPFLAGS = $(AM_CPPFLAGS) -DBUILDING_URBI_SDK -DBUILDING_LIBPORT
libuobject_la_LDFLAGS = $(BOOST_THREAD_LDFLAGS) -no-undefined
libuobject_la_LIBADD = $(PTHREAD_LIBS) $(LTDL_LIBS) $(top_builddir)/lib/libport/libport.la
# The separation in two different targets is needed to be sure that
# the libs are installed before we try to move them.
# install-exec-local:

#install-data-hook:
#	rm $(DESTDIR)$(envdir)/.libs
#	$(LN_S) . $(DESTDIR)$(envdir)/.libs
#	$(mkdir_p) $(DESTDIR)$(envdir)/.libs
#	mv $(DESTDIR)$(envdir)/lib* $(DESTDIR)$(envdir)/.libs
#	$(LN_S) .libs/libuobject.la $(DESTDIR)$(envdir)
#	$(LN_S) .libs/libuobject.a $(DESTDIR)$(envdir)

#uninstall-local:
#	rm -rf $(DESTDIR)$(envdir)/.libs

## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/build-aux/revision.mk

kernelinclude_HEADERS =
dist_libuobject_la_SOURCES =
nodist_libuobject_la_SOURCES =
BUILT_SOURCES += $(nodist_libuobject_la_SOURCES)
# Needed in object/local.mk, used potentially everywhere else (maybe
# before, so initialize it here).
precompiled_symbols_hh_deps =

## ---------- ##
## uobject/.  ##
## ---------- ##

include $(top_srcdir)/build-aux/svn-externals.mk

include uobject/uobject.mk
dist_libuobject_la_SOURCES +=				\
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

bin_PROGRAMS=
if !OPENR
bin_PROGRAMS += urbi-console
if !WIN32
bin_PROGRAMS += ast-dump
ast_dump_SOURCES = ast-dump.cc ast/dot-printer.cc
ast_dump_LDADD = libuobject.la $(PTHREAD_LIBS) $(LTDL_LIBS) \
        $(BOOST_REGEX_LDFLAGS) $(BOOST_REGEX_LIBS)
endif

urbi_console_SOURCES = urbi-console.cc
urbi_console_LDADD = libuobject.la $(PTHREAD_LIBS) $(LTDL_LIBS)

dist_libuobject_la_SOURCES +=			\
console-server.cc				\
../lib/network/bsdnet/connection.hh		\
../lib/network/bsdnet/connection.cc		\
../lib/network/bsdnet/network.hh		\
../lib/network/bsdnet/network.cc


endif


if ! INSTALL_KERNEL_HEADERS
install-data-hook: remove_kheaders
remove_kheaders:
	$(RM) -r $(prefix)/gostai/kernel
endif
