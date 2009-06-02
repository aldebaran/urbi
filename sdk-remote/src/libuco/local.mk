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

dist_libuco_libuco_la_SOURCES =			\
  libuco/exit.cc				\
  libuco/gd-init.cc				\
  libuco/package-info.cc			\
  libuco/shared-library-checker.cc		\
  libuco/uimage.cc				\
  libuco/uobject-common.cc			\
  libuco/uobject-hub-common.cc			\
  libuco/uprop.cc				\
  libuco/urbi-main.cc				\
  libuco/ustarter.cc				\
  libuco/utable.cc				\
  libuco/uvalue-common.cc			\
  libuco/uvar-common.cc
