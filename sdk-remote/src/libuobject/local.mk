## --------------- ##
## libuobject.la.  ##
## --------------- ##

env_LTLIBRARIES = libuobject/libuobject.la
libuobject_libuobject_la_SOURCES =		\
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
libuobject_libuobject_la_LDFLAGS = -no-undefined

# We might need to have two versions of libuobject: one to link
# against, and the other one to dlopen.  It seems that we can live
# with a single one for the moment, but who knows...
#
# env_LTLIBRARIES += libuobject/uobject.la
# libuobject_uobject_la_SOURCES  = $(libuobject_libuobject_la_SOURCES)
# libuobject_uobject_la_CPPFLAGS = $(libuobject_libuobject_la_CPPFLAGS)
# libuobject_uobject_la_LIBADD   = $(libuobject_libuobject_la_LIBADD)
# libuobject_uobject_la_LDFLAGS  = $(libuobject_libuobject_la_LDFLAGS) -module

all-local: libuobject/libuobject.la.stamp

libuobject/libuobject.la.stamp: libuobject/libuobject.la
	$(build_aux_dir)/fix-libtool-la libuobject/libuobject.la \
	  libuobject/libuobject.la.stamp

CLEANFILES += libuobject/libuobject.la.stamp
