## Copyright (C) 2008-2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## --------------- ##
## libuobject.la.  ##
## --------------- ##

# Hook to install-exec, not install-data.
execremotedir = $(remotedir)
execremote_LTLIBRARIES = libuobject/libuobject@LIBSFX@.la
libuobject_libuobject@LIBSFX@_la_SOURCES =	\
  libuobject/main.cc				\
  libuobject/remote-ucontext-impl.hh		\
  libuobject/ucallbacks.cc			\
  libuobject/uobject.cc				\
  libuobject/utimer-callback.cc			\
  libuobject/uvar.cc
libuobject_libuobject@LIBSFX@_la_CPPFLAGS =	\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK
libuobject_libuobject@LIBSFX@_la_LIBADD =	\
  $(LIBADD)					\
  libuco/libuco.la				\
  liburbi/liburbi$(LIBSFX).la			\
  $(BOOST_DATE_TIME_LIBS)			\
  $(SERIALIZE_LIBS)
libuobject_libuobject@LIBSFX@_la_LDFLAGS =	\
  -avoid-version -no-undefined			\
  $(BOOST_DATE_TIME_LDFLAGS)

# libuobject depends on liburbi, and make install installs them (and
# therefore relinks them) in unspecified order.  So be sure to install
# lib libraries before the env libraries.  We cannot insert the
# dependency here, because Automake thinks we are trying to override
# its definition of install-execremotedir and no longer produces it.  As
# a result, we do have the right dependencies, but nothing is
# installed.
install_execremoteltlibraries = install-execremoteLTLIBRARIES
$(install_execremoteltlibraries): install-libLTLIBRARIES
