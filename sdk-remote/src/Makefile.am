include $(top_srcdir)/build-aux/init.mk

SUBDIRS = . tests
if BINDINGS
 SUBDIRS += swig
endif

# Find libport's sources and config.h.
AM_CPPFLAGS += $(LIBPORT_CPPFLAGS)
# Find libjpeg's source and build (jconfig.h) headers.
AM_CPPFLAGS += -I$(top_srcdir) -I$(top_builddir)/jpeg-6b
# Find urbi/ headers.
AM_CPPFLAGS += -I$(top_srcdir)/include
# Find version.hh.
AM_CPPFLAGS += -I$(top_builddir)/src
# Find sdk/config.h
AM_CPPFLAGS += -I$(top_builddir)

AM_CXXFLAGS += $(PTHREAD_CFLAGS) $(SOCKETS_CFLAGS) $(WARNING_CXXFLAGS)

LIBADD       =					\
  $(PTHREAD_LIBS) $(SOCKETS_LIBS)		\
  $(top_builddir)/lib/libport/libport.la
AUTOMAKE_OPTIONS += subdir-objects

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

# To be linked in all the executables (urbi-launch, examples/ etc.).
AM_LDADD =					\
  liburbi/liburbi.la				\
  $(top_builddir)/jpeg-6b/libjpeg.la		\
  $(PTHREAD_LIBS)				\
  $(top_builddir)/lib/libport/libport.la

include examples/local.mk
include libuco/local.mk
include liburbi/local.mk
include libuobject/local.mk
include utils/local.mk
## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/build-aux/revision.mk

nodist_env_DATA = version.hh

## ------------- ##
## Urbi-launch.  ##
## ------------- ##

bin_PROGRAMS += urbi-launch
urbi_launch_SOURCES = urbi-launch.cc
# Find our own ltdl.h.  Push "system" headers last so that our headers
# have priority.
urbi_launch_CPPFLAGS = $(LTDL_CPPFLAGS) $(AM_CPPFLAGS)
urbi_launch_LDADD =				\
  $(AM_LDADD)					\
  $(LTDL_LIBS)					\
  liburbi/liburbi.la

urbi_launch_LDFLAGS =				\
  $(AM_LDFLAGS)
