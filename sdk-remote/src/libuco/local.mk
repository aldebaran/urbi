## Copyright (C) 2008-2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## --------------------- ##
## Sources and headers.  ##
## --------------------- ##

noinst_LTLIBRARIES += libuco/libuco.la
libuco_libuco_la_CPPFLAGS =			\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK
libuco_libuco_la_LDFLAGS = $(BOOST_THREAD_LDFLAGS) $(BOOST_DATE_TIME_LDFLAGS)
libuco_libuco_la_LIBADD =			\
  libuvalue/libuvalue.la			\
  $(BOOST_THREAD_LIBS)				\
  $(BOOST_DATE_TIME_LIBS)

dist_libuco_libuco_la_SOURCES =			\
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
