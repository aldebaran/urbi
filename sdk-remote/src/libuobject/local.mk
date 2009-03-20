## --------------- ##
## libuobject.la.  ##
## --------------- ##

env_LTLIBRARIES = libuobject/libuobject.la
libuobject_libuobject_la_SOURCES =		\
  libuobject/compatibility.hh			\
  libuobject/compatibility.hxx			\
  libuobject/main.cc				\
  libuobject/ucallbacks.cc			\
  libuobject/uobject.cc				\
  libuobject/usystem.cc				\
  libuobject/utimer-callback.cc			\
  libuobject/uvar.cc
libuobject_libuobject_la_CPPFLAGS =		\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK
libuobject_libuobject_la_LIBADD = $(LIBADD) libuco/libuco.la liburbi/liburbi.la
libuobject_libuobject_la_LDFLAGS = -avoid-version -no-undefined
