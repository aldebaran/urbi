## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = libuco/version.hh
include $(top_srcdir)/build-aux/revision.mk

nodist_env_DATA = $(REVISION_FILE)

## --------------------- ##
## Sources and headers.  ##
## --------------------- ##

noinst_LTLIBRARIES += libuco/libuco.la
libuco_libuco_la_CPPFLAGS =			\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK
# Find version.hh.
libuco_libuco_la_CPPFLAGS += -I.
libuco_libuco_la_LDFLAGS = $(BOOST_THREAD_LDFLAGS)
libuco_libuco_la_LIBADD = $(BOOST_THREAD_LIBS)

dist_libuco_libuco_la_SOURCES =			\
  libuco/exit.cc				\
  libuco/gd-init.cc				\
  libuco/package-info.cc			\
  libuco/uimage.cc				\
  libuco/uobject-common.cc			\
  libuco/uobject-hub-common.cc			\
  libuco/uprop.cc				\
  libuco/usound.cc				\
  libuco/ustarter.cc				\
  libuco/utable.cc				\
  libuco/uvalue-common.cc			\
  libuco/uvar-common.cc
