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
libuco_libuco_la_LIBADD = libuvalue/libuvalue.la $(BOOST_THREAD_LIBS)

dist_libuco_libuco_la_SOURCES =			\
  libuco/gd-init.cc				\
  libuco/uevent.cc				\
  libuco/uobject-common.cc			\
  libuco/uobject-hub-common.cc			\
  libuco/uprop.cc				\
  libuco/urbi-main.cc				\
  libuco/ustarter.cc				\
  libuco/utable.cc				\
  libuco/uvar-common.cc


## ---------- ##
## umain.cc.  ##
## ---------- ##

dist_umain_DATA = liburbi/umain.cc liburbi/urbi-root.cc

