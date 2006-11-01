SUBDIRS =
if BINDINGS
 SUBDIRS += swig
endif
SUBDIRS += .

# Initialize some common variables.
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST =
MAINTAINERCLEANFILES =

# Find libjpeg's src (jpeglib.h) and build (jconfig.h) files.
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)/jpeg-6b

AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)
AM_LIBADD = $(PTHREAD_LIBS)


## --------- ##
## uobject.  ##
## --------- ##

include uobject/uobject.mk

## ------------ ##
## liburbi.la.  ##
## ------------ ##

lib_LTLIBRARIES = liburbi.la

include_HEADERS =				\
uabstractclient.h				\
uclient.h					\
uconversion.h					\
uexternal.h					\
uobject/uobject.hh				\
usyncclient.h

sdkinclude_HEADERS = 				\
uobject/uobject.hh

liburbi_la_SOURCES =				\
$(uobject_sources)				\
uabstractclient.h	uabstractclient.cpp	\
uclient.h		uclient.cpp		\
uconversion.h		uconversion.cpp		\
uexternal.h					\
uobject.cc					\
uvar.cc						\
usyncclient.h		usyncclient.cpp
liburbi_la_LIBADD = $(top_builddir)/jpeg-6b/libjpeg.la $(PTHREAD_LIBS)


## ------------ ##
## liburbi.la.  ##
## ------------ ##

env_LTLIBRARIES = libkernel-remote.la
libkernel_remote_la_SOURCES = $(liburbi_la_SOURCES) $(top_srcdir)/sdk/umain.cc
libkernel_remote_la_LIBADD = $(AM_LIBADD) $(top_builddir)/jpeg-6b/libjpeg.la
