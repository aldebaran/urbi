## --------------- ##
## libuobject.la.  ##
## --------------- ##

env_LTLIBRARIES = libuobject/libuobject.la
libuobject_libuobject_la_SOURCES =		\
  libuobject/package-info.cc			\
  libuobject/ucallbacks.cc			\
  libuobject/uobject.cc				\
  libuobject/usystem.cc				\
  libuobject/utimer-callback.cc			\
  libuobject/uvar.cc
libuobject_libuobject_la_CPPFLAGS =		\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK -DBUILDING_LIBPORT
libuobject_libuobject_la_LIBADD = $(LIBADD) libuco/libuco.la liburbi/liburbi.la
libuobject_libuobject_la_LDFLAGS = -no-undefined

all-local: libuobject/libuobject.la.stamp

libuobject/libuobject.la.stamp: libuobject/libuobject.la
	$(top_srcdir)/build-aux/fix-libtool-la libuobject/libuobject.la \
	  libuobject/libuobject.la.stamp

CLEANFILES += libuobject/libuobject.la.stamp
