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
libuobject_libuobject_la_LDFLAGS = -avoid-version -no-undefined

# libuobject depends on liburbi, and make install installs them (and
# therefore relinks them) in unspecified order.  So be sure to install
# lib libraries before the env libraries.
install-envLTLIBRARIES: install-libLTLIBRARIES

