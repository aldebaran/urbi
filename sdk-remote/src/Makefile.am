include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/fix-libtool-la.mk
AUTOMAKE_OPTIONS += subdir-objects

SUBDIRS = . tests
if BINDINGS
 SUBDIRS += swig
endif

LTDL_LIBS = $(top_builddir)/libport/libltdl/libltdlc.la
LTDL_CPPFLAGS = -I$(top_srcdir)/libport -I$(top_builddir)/libport

LIBPORT_CPPFLAGS =				\
  -I$(top_srcdir)/libport/include		\
  -I$(top_builddir)/libport/include

JPEG6B_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)/jpeg-6b

AM_CPPFLAGS +=					\
  $(LIBPORT_CPPFLAGS)				\
  $(JPEG6B_CPPFLAGS)				\
  -I$(top_srcdir)/include -I$(top_builddir)

AM_CXXFLAGS += $(PTHREAD_CFLAGS) $(SOCKETS_CFLAGS) $(WARNING_CXXFLAGS)

LIBADD =					\
  $(PTHREAD_LIBS) $(SOCKETS_LIBS)		\
  $(top_builddir)/libport/lib/libport/libport.la

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

# To be linked in all the executables (urbi-launch, examples/ etc.).
AM_LDADD =					\
  liburbi/liburbi.la				\
  $(top_builddir)/jpeg-6b/libjpeg.la		\
  $(PTHREAD_LIBS)				\
  $(top_builddir)/libport/lib/libport/libport.la

include libuco/local.mk
include liburbi/local.mk
include libuobject/local.mk
include uobjects/local.mk

if EXAMPLES
include examples/local.mk
include utils/local.mk
endif EXAMPLES


## ---------------------- ##
## Convenient shortcuts.  ##
## ---------------------- ##

.PHONY: core libs
libs: $(LTLIBRARIES)
core: libs urbi-launch$(EXEEXT)

## ------------- ##
## Urbi-launch.  ##
## ------------- ##

bin_PROGRAMS += urbi-launch
# Find our own ltdl.h.  Push "system" headers last so that our headers
# have priority.
urbi_launch_CPPFLAGS = $(LTDL_CPPFLAGS) $(AM_CPPFLAGS)
urbi_launch_SOURCES = urbi-launch.cc
urbi_launch_LDFLAGS =				\
  $(AM_LDFLAGS)
urbi_launch_LDADD =				\
  $(AM_LDADD)					\
  $(LTDL_LIBS)					\
  liburbi/liburbi.la
