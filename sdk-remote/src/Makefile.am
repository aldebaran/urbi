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
include libuobject/local.mk

if SDK_REMOTE
include bin/local.mk
include liburbi/local.mk
include uobjects/local.mk
endif SDK_REMOTE

if EXAMPLES
include examples/local.mk
include utils/local.mk
endif EXAMPLES


## ---------------------- ##
## Convenient shortcuts.  ##
## ---------------------- ##

.PHONY: core libs
libs: $(LTLIBRARIES)
core: libs bin/urbi-launch$(EXEEXT)
