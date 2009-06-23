include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/fix-libtool-la.mk
include $(top_srcdir)/build-aux/libport.mk

AUTOMAKE_OPTIONS += subdir-objects

SUBDIRS = . tests
if BINDINGS
 SUBDIRS += swig
endif

# libjpeg.
JPEG_CPPFLAGS = -I$(top_srcdir)/jpeg -I$(top_builddir)/jpeg
JPEG_LIBS = $(top_builddir)/jpeg/libjpeg.la

AM_CPPFLAGS +=					\
  $(LIBPORT_CPPFLAGS)				\
  $(JPEG_CPPFLAGS)				\
  -I$(top_srcdir)/include -I$(top_builddir)	\
  -I$(srcdir)

AM_CXXFLAGS += $(PTHREAD_CFLAGS) $(SOCKETS_CFLAGS) $(WARNING_CXXFLAGS)

LIBADD =					\
  $(PTHREAD_LIBS)				\
  $(SOCKETS_LIBS)				\
  $(LIBPORT_LIBS)

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

# To be linked in all the executables (urbi-launch, examples/ etc.).
AM_LDADD =					\
  liburbi/liburbi.la				\
  $(JPEG_LIBS)					\
  $(PTHREAD_LIBS)				\
  $(LIBPORT_LIBS)

include libuco/local.mk

if SDK_REMOTE
include liburbi/local.mk
include uobjects/local.mk
endif SDK_REMOTE

# libuobject depends on liburbi, and make install installs them (and
# therefore relinks them) in the order of definition.  So be sure to
# load liburbi/local.mk before libuobject/local.mk.
include libuobject/local.mk

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

if SDK_REMOTE
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
endif
