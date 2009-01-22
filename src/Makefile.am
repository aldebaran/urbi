## ---------------- ##
## Initialization.  ##
## ---------------- ##

include $(top_srcdir)/build-aux/init.mk
## Get $(libport_la).
## lib_libport = ../lib/libport
## include $(top_srcdir)/lib/libport/libport-sources.mk

## Find the kernel private headers.  location.hh etc. are generated.
AM_CPPFLAGS += -I$(srcdir) -I.

## Find the kernel, libport, etc. headers.
AM_CPPFLAGS +=					\
  -I$(top_srcdir)/include			\
  -I$(top_builddir)/include

## Headers exported by SDK-Remote.
AM_CPPFLAGS +=					\
  -I$(top_srcdir)/sdk-remote/include		\
  -I$(top_builddir)/sdk-remote/include		\
  -I$(top_builddir)/sdk-remote

## src/include/sched etc.
AM_CPPFLAGS += -I$(srcdir)/include

## network has its includes in the same dir as its source files.
AM_CPPFLAGS += -I$(srcdir)/lib

AM_CXXFLAGS += $(WARNING_CXXFLAGS) $(PTHREAD_CFLAGS)

## Add boost libraries.
AM_CPPFLAGS += $(BOOST_CPPFLAGS)

## ------------ ##
## libuobject.  ##
## ------------ ##

env_LTLIBRARIES = libuobject.la
# The -DBUILDING_LIBPORT is required since libport/network/* are
# considered as kernel sources.
libuobject_la_CPPFLAGS =				\
  -I$(top_srcdir)/sdk-remote/libltdl			\
  $(AM_CPPFLAGS) -DBUILDING_URBI_SDK
libuobject_la_LDFLAGS =					\
  -avoid-version -no-undefined
libuobject_la_LIBADD =						\
  $(SOCKETS_LIBS)						\
  $(PTHREAD_LIBS)						\
  $(top_builddir)/sdk-remote/libltdl/libltdlc.la		\
  $(top_builddir)/sdk-remote/lib/libport/libport.la		\
  $(top_builddir)/sdk-remote/src/libuco/libuco.la		\
  lib/sched/libsched.la


## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/build-aux/revision.mk
dist_noinst_HEADERS = $(REVISION_FILE)

dist_libuobject_la_SOURCES =
nodist_libuobject_la_SOURCES =
BUILT_SOURCES += $(nodist_libuobject_la_SOURCES)
# Needed in object/local.mk, used potentially everywhere else (maybe
# before, so initialize it here).
precompiled_symbols_hh_deps =


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
include kernel/local.mk
libport_la = $(top_builddir)/sdk-remote/lib/libport/libport.la
include lib/sched/local.mk
include urbi/local.mk

urbiinclude_HEADERS = $(uobject_headers)

## ---------- ##
## programs.  ##
## ---------- ##

# urbi-console.
bin_PROGRAMS += urbi-console

urbi_console_SOURCES = urbi-console.cc
urbi_console_LDADD =				\
  libuobject.la					\
  $(libuobject_la_LIBADD)
dist_libuobject_la_SOURCES +=			\
  console-server.cc				\
  lib/network/bsdnet/connection.hh		\
  lib/network/bsdnet/connection.cc		\
  lib/network/bsdnet/network.hh			\
  lib/network/bsdnet/network.cc

all-local: libuobject.la.stamp

libuobject.la.stamp: libuobject.la
	$(top_srcdir)/build-aux/fix-libtool-la libuobject.la \
	  libuobject.la.stamp

CLEANFILES += libuobject.la.stamp

if BUILD_PROGRAMS
if !WIN32

# ast-dump.
noinst_PROGRAMS = ast-dump
dist_ast_dump_SOURCES =				\
  ast-dump.cc
ast_dump_CPPFLAGS = $(libuobject_la_CPPFLAGS)
ast_dump_LDADD    =		\
  libuobject.la
#  $(libuobject_la_LIBADD)

endif !WIN32
endif BUILD_PROGRAMS

# We used to install a different version (with s/remote/engine/g), but
# there is no occurrence of remote in param.mk.  Yet MN insists on
# installing this guy.
nodist_env_DATA = $(top_builddir)/sdk-remote/sdk/param.mk

if ! INSTALL_KERNEL_HEADERS
install-data-hook: remove_kheaders

remove_kheaders:
	$(RM) -r $(prefix)/gostai/kernel
endif
