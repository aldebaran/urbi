include $(top_srcdir)/flags.mk

# Path to envdir from top/install_prefix
topenvdir = $(PACKAGE_BRAND)/core/$(URBI_HOST)/$(URBI_ENV)

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

include libuco/local.mk

## ------------ ##
## liburbi.la.  ##
## ------------ ##

umaindir = $(datadir)/umain
dist_umain_DATA = umain.cc

env_LTLIBRARIES = libuobject.la
lib_LTLIBRARIES = liburbi.la

libuobject_la_SOURCES =				\
  package-info.cc				\
  ucallbacks.cc					\
  uobject.cc					\
  usystem.cc					\
  utimer-callback.cc				\
  uvar.cc

liburbi_la_SOURCES =				\
  libuco/uvalue-common.cc			\
  uabstractclient.cc				\
  uclient.cc					\
  uconversion.cc				\
  usyncclient.cc


liburbi_la_CPPFLAGS = $(AM_CPPFLAGS) -DBUILDING_URBI_SDK -DBUILDING_LIBPORT
liburbi_la_LIBADD = $(LIBADD)
liburbi_la_LDFLAGS = -no-undefined

libuobject_la_CPPFLAGS = $(AM_CPPFLAGS) -DBUILDING_URBI_SDK -DBUILDING_LIBPORT
libuobject_la_LIBADD = $(LIBADD) libuco/libuco.la liburbi.la
libuobject_la_LDFLAGS = -no-undefined



bin_PROGRAMS = urbi-launch
urbi_launch_SOURCES = urbi-launch.cc
urbi_launch_CPPFLAGS = $(AM_CPPFLAGS) $(BOOST_CPPFLAGS)
urbi_launch_LDADD =				\
  $(AM_LDADD)					\
  $(BOOST_FILESYSTEM_LIBS) $(BOOST_SYSTEM_LIBS)	\
  $(LTDL_LIBS)					\
  liburbi.la
urbi_launch_LDFLAGS =					\
  $(AM_LDFLAGS)						\
  $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS)
