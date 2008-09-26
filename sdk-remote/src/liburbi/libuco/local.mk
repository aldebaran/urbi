## --------------------- ##
## Sources and headers.  ##
## --------------------- ##

noinst_LTLIBRARIES = libuco/libuco.la
libuco_libuco_la_CPPFLAGS = $(AM_CPPFLAGS) -DBUILDING_URBI_SDK -DBUILDING_LIBPORT

dist_libuco_libuco_la_SOURCES =			\
  libuco/uobject-hub-common.cc			\
  libuco/utable.cc				\
  libuco/utimer-table.cc			\
  libuco/uvalue-common.cc			\
  libuco/urbi-main.cc				\
  libuco/uvar-common.cc
