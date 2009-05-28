## ---------------- ##
## Initialization.  ##
## ---------------- ##

include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/fix-libtool-la.mk
include $(top_srcdir)/build-aux/libport.mk

# A tool that cuts code we don't want to keep in space-optimized
# versions (when cpp cannot be used).
PRUNE_FOR_SPACE = $(build_aux_dir)/studentize --mark=no-space
if !OPTIMIZE_SPACE
  PRUNE_FOR_SPACE += --stage=no-space
endif


## Find the kernel private headers.  location.hh etc. are generated.
AM_CPPFLAGS += -I$(srcdir) -I.

## Find the kernel, libport, etc. headers.
AM_CPPFLAGS +=					\
  -I$(top_srcdir)/include			\
  -I$(top_builddir)/include			\
  $(LIBPORT_CPPFLAGS)				\
  $(LTDL_CPPFLAGS)

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
  $(AM_CPPFLAGS) -DBUILDING_URBI_SDK -DLIBPORT_NO_SSL
libuobject_la_LDFLAGS =				\
  -avoid-version -no-undefined
libuobject_la_LIBADD =					\
  $(SOCKETS_LIBS)					\
  $(PTHREAD_LIBS)					\
  $(LTDL_LIBS)						\
  $(LIBPORT_LIBS)					\
  $(top_builddir)/sdk-remote/src/libuco/libuco.la	\
  $(BOOST_SYSTEM_LIBS)					\
  $(SCHED_LIBS)


## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/build-aux/revision.mk
dist_noinst_HEADERS = $(REVISION_FILE)

dist_libuobject_la_SOURCES =
nodist_libuobject_la_SOURCES =
BUILT_SOURCES += $(nodist_libuobject_la_SOURCES)
CLEANFILES += $(nodist_libuobject_la_SOURCES)
# Needed in object/local.mk, used potentially everywhere else (maybe
# before, so initialize it here).
precompiled_symbols_hh_deps =


## ----------------- ##
## Sub directories.  ##
## ----------------- ##

include ast/local.mk
include bin/local.mk
include binder/local.mk
include flower/local.mk
include parser/local.mk
include object/local.mk
include rewrite/local.mk
include runner/local.mk
include kernel/local.mk
include lib/sched/local.mk
include urbi/local.mk

urbiinclude_HEADERS = $(uobject_headers)

dist_libuobject_la_SOURCES +=			\
  console-server.cc				\
  lib/network/bsdnet/connection.hh		\
  lib/network/bsdnet/connection.cc		\
  lib/network/bsdnet/network.hh			\
  lib/network/bsdnet/network.cc

# We used to install a different version (with s/remote/engine/g), but
# there is no occurrence of remote in param.mk.  Yet MN insists on
# installing this guy.
nodist_env_DATA = $(top_builddir)/sdk-remote/sdk/param.mk
