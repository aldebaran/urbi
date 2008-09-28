## ---------------- ##
## Initialization.  ##
## ---------------- ##

include kernel.mk
AUTOMAKE_OPTIONS += -Wno-portability subdir-objects


## ------------ ##
## libuobject.  ##
## ------------ ##

env_LTLIBRARIES = libuobject.la
# The -DBUILDING_LIBPORT is required since libport/network/* are
# considered as kernel sources.
libuobject_la_CPPFLAGS =				\
  $(AM_CPPFLAGS) -DBUILDING_URBI_SDK -DBUILDING_LIBPORT
libuobject_la_LDFLAGS =					\
  $(BOOST_THREAD_LDFLAGS)				\
  $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)	\
  -no-undefined
libuobject_la_LIBADD =						\
  $(PTHREAD_LIBS)						\
  $(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)			\
  $(LTDL_LIBS)							\
  $(top_builddir)/sdk-remote/lib/libport/libport.la		\
  $(top_builddir)/sdk-remote/src/liburbi/libuco/libuco.la


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
include scheduler/local.mk
include scheduler/libcoroutine/local.mk
include kernel/local.mk

urbiinclude_HEADERS = $(uobject_headers)

## ---------- ##
## programs.  ##
## ---------- ##

bin_PROGRAMS =
if !OPENR

# urbi-console.
bin_PROGRAMS += urbi-console

urbi_console_SOURCES = urbi-console.cc
urbi_console_LDADD = libuobject.la
dist_libuobject_la_SOURCES +=			\
console-server.cc				\
../lib/network/bsdnet/connection.hh		\
../lib/network/bsdnet/connection.cc		\
../lib/network/bsdnet/network.hh		\
../lib/network/bsdnet/network.cc

if !WIN32
# ast-dump.
bin_PROGRAMS += ast-dump
dist_ast_dump_SOURCES = ast-dump.cc ast/dot-printer.cc
ast_dump_LDFLAGS = $(BOOST_REGEX_LDFLAGS)
ast_dump_LDADD   = $(BOOST_REGEX_LIBS) libuobject.la
endif !WIN32
endif !OPENR


if ! INSTALL_KERNEL_HEADERS
install-data-hook: remove_kheaders
remove_kheaders:
	$(RM) -r $(prefix)/gostai/kernel
endif
