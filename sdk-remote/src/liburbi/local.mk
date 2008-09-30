# Path to envdir from top/install_prefix
topenvdir = $(PACKAGE_BRAND)/core/$(URBI_HOST)/$(URBI_ENV)

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

## ------------ ##
## liburbi.la.  ##
## ------------ ##

umaindir = $(datadir)/umain
dist_umain_DATA = liburbi/umain.cc

env_LTLIBRARIES = liburbi/libuobject.la
liburbi_libuobject_la_SOURCES =			\
  liburbi/package-info.cc			\
  liburbi/ucallbacks.cc				\
  liburbi/uobject.cc				\
  liburbi/usystem.cc				\
  liburbi/utimer-callback.cc			\
  liburbi/uvar.cc
liburbi_libuobject_la_CPPFLAGS =		\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK -DBUILDING_LIBPORT
liburbi_libuobject_la_LIBADD = $(LIBADD) libuco/libuco.la liburbi/liburbi.la
liburbi_libuobject_la_LDFLAGS = -no-undefined


lib_LTLIBRARIES = liburbi/liburbi.la
liburbi_liburbi_la_SOURCES =			\
  libuco/uvalue-common.cc			\
  liburbi/uabstractclient.cc			\
  liburbi/uclient.cc				\
  liburbi/uconversion.cc			\
  liburbi/usyncclient.cc
liburbi_liburbi_la_CPPFLAGS =			\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK -DBUILDING_LIBPORT
liburbi_liburbi_la_LIBADD = $(LIBADD)
liburbi_liburbi_la_LDFLAGS = -no-undefined



bin_PROGRAMS += liburbi/urbi-launch
liburbi_urbi_launch_SOURCES = liburbi/urbi-launch.cc
liburbi_urbi_launch_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS)
liburbi_urbi_launch_LDADD =			\
  $(AM_LDADD)					\
  $(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)	\
  $(LTDL_LIBS)					\
  liburbi/liburbi.la
liburbi_urbi_launch_LDFLAGS =				\
  $(AM_LDFLAGS)						\
  $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)
