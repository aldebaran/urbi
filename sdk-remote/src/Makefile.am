include $(top_srcdir)/flags.mk
AUTOMAKE_OPTIONS += subdir-objects

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

AM_LDADD =					\
  liburbi/liburbi.la				\
  $(top_builddir)/jpeg-6b/libjpeg.la		\
  $(PTHREAD_LIBS)

include examples/local.mk
include libuco/local.mk
include liburbi/local.mk
include libuobject/local.mk
include utils/local.mk

SUBDIRS = . tests
if BINDINGS
 SUBDIRS += swig
endif

## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = version.hh
include $(top_srcdir)/build-aux/revision.mk


bin_PROGRAMS += urbi-launch
urbi_launch_SOURCES = urbi-launch.cc
urbi_launch_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS)
# Find our own ltdl.h.
urbi_launch_CPPFLAGS += -I$(top_srcdir)/libltdl
urbi_launch_LDADD =				\
  $(AM_LDADD)					\
  $(top_builddir)/libltdl/libltdlc.la		\
  liburbi/liburbi.la
urbi_launch_LDFLAGS =				\
  $(AM_LDFLAGS)
