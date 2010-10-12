## Copyright (C) 2008-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## ------------ ##
## liburbi.la.  ##
## ------------ ##

lib_LTLIBRARIES += liburbi/liburbi@LIBSFX@.la
# FIXME: Something is fishy here: why do we duplicate libuco code in
# both liburbi and libuobject?  Libuobject includes liburbi.
liburbi_liburbi@LIBSFX@_la_SOURCES =		\
  liburbi/compatibility.hh			\
  liburbi/compatibility.hxx			\
  liburbi/kernel-version.cc			\
  liburbi/uabstractclient.cc			\
  liburbi/uclient.cc				\
  liburbi/uconversion.cc			\
  liburbi/umessage.cc				\
  liburbi/urbi-launch.cc			\
  liburbi/usyncclient.cc			\
  liburbi/utag.cc

liburbi_liburbi@LIBSFX@_la_CPPFLAGS =		\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK
# Find version.hh.
liburbi_liburbi@LIBSFX@_la_CPPFLAGS += -I. -DURBI_ROOT_NOT_DLL
# We are using Libport.OptionParser which uses Boost.Optional, which is
# incompatible with strict alias analysis.
liburbi_liburbi@LIBSFX@_la_CXXFLAGS = -fno-strict-aliasing $(AM_CXXFLAGS)
liburbi_liburbi@LIBSFX@_la_LIBADD =		\
  $(LIBADD)					\
  libuvalue/libuvalue.la			\
  $(JPEG_LIBS)
liburbi_liburbi@LIBSFX@_la_LDFLAGS = -avoid-version -no-undefined
