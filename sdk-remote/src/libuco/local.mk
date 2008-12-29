## --------------------- ##
## Sources and headers.  ##
## --------------------- ##

noinst_LTLIBRARIES += libuco/libuco.la
libuco_libuco_la_CPPFLAGS =			\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK

dist_libuco_libuco_la_SOURCES =			\
  libuco/uobject-hub-common.cc			\
  libuco/uprop.cc				\
  libuco/ustarter.cc				\
  libuco/utable.cc				\
  libuco/uvalue-common.cc			\
  libuco/urbi-main.cc				\
  libuco/uvar-common.cc
