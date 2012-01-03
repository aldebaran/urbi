## Copyright (C) 2006-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## ---------------- ##
## Initialization.  ##
## ---------------- ##

include $(top_srcdir)/build-aux/make/init.mk
include $(top_srcdir)/build-aux/make/fix-libtool-la.mk
include $(top_srcdir)/build-aux/make/libport.mk

# A tool that cuts code we don't want to keep in space-optimized
# versions (when cpp cannot be used).
PRUNE_FOR_SPACE = $(build_aux_dir)/bin/studentize --mark=no-space
if !COMPILATION_MODE_SPACE
  PRUNE_FOR_SPACE += --stage=no-space
endif


## Find the kernel private headers.  location.hh etc. are generated.
AM_CPPFLAGS += -I$(srcdir) -I.

## Find the kernel, libport, etc. headers.
AM_CPPFLAGS +=					\
  -I$(top_srcdir)/include			\
  -I$(top_builddir)/include			\
  -D__SRCDIR__="\"$(top_srcdir)\""		\
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

engine_LTLIBRARIES = libuobject@LIBSFX@.la
libuobject@LIBSFX@_la_CPPFLAGS =		\
  $(AM_CPPFLAGS) 				\
  -DBUILDING_URBI_SDK 				\
  -DLIBPORT_NO_SSL

# Unfortunately, we _have_ to disable -fstrict-aliasing.  Because of
# Bison's variants.  I don't know how to avoid this.
#
#   In file included from parser/ugrammar.cc:41:
#   parser/ugrammar.hh: In member function 'T& yy::variant<S>::as() [with T = libport::intrusive_ptr<ast::Nary>, unsigned int S = 28u]':
#   parser/ugrammar-pruned.y:404:   instantiated from here
#   parser/ugrammar.hh:135: error: dereferencing type-punned pointer will break strict-aliasing rules
#
libuobject@LIBSFX@_la_CXXFLAGS = $(AM_CXXFLAGS) -fno-strict-aliasing

libuobject@LIBSFX@_la_LDFLAGS =				\
  -avoid-version -no-undefined
libuobject@LIBSFX@_la_LIBADD =				\
  $(BOOST_DATE_TIME_LIBS)				\
  $(LIBPORT_LIBS)					\
  $(SCHED_LIBS)						\
  $(SERIALIZE_LIBS)					\
  $(top_builddir)/sdk-remote/src/libuco/libuco.la

## ---------------------- ##
## Revision information.  ##
## ---------------------- ##

# Resolve stubs in libuobject.
all-local: libuobject.unstub.stamp
VERSIONIFY_STUBS = libuobject$(LIBSFX).la
libuobject.unstub.stamp: $(VERSIONIFY_STUBS) $(VERSIONIFY_CACHE) $(VERSIONIFY)
	$(AT_V_PATCH) $(VERSIONIFY_RUN)		\
	  $(if $(V:0=),-v,-q)			\
	  $(addprefix --resolve=, $(VERSIONIFY_STUBS))

dist_libuobject@LIBSFX@_la_SOURCES =
nodist_libuobject@LIBSFX@_la_SOURCES =
BUILT_SOURCES += $(nodist_libuobject@LIBSFX@_la_SOURCES)
CLEANFILES += $(nodist_libuobject@LIBSFX@_la_SOURCES)
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
include urbi/local.mk

urbiinclude_HEADERS = $(uobject_headers)

dist_libuobject@LIBSFX@_la_SOURCES +=			\
  console-server.cc

# We used to install a different version (with s/remote/engine/g), but
# there is no occurrence of remote in param.mk.  Yet MN insists on
# installing this guy.
nodist_engine_DATA = $(top_builddir)/sdk-remote/sdk/param.mk
