if OPENR
SUBDIRS = openr .
else
SUBDIRS = uobject .
endif 

AM_CPPFLAGS = \
-I$(top_srcdir) \
-I$(top_builddir)/src/liburbi/uobject \
-I$(top_srcdir)/src/liburbi/uobject

AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)
AM_LIBADD =  $(PTHREAD_LIBS)

## ------------ ##
## liburbi.la.  ##
## ------------ ##

lib_LTLIBRARIES = liburbi.la

include_HEADERS =				\
uabstractclient.h				\
uclient.h					\
uconversion.h					\
uobject/uext.h					\
uobject/uexternal.h				\
uobject/uobject.h				\
uobject/uext.h                                  \
usyncclient.h

sdkinclude_HEADERS = 				\
uobject/uext.h					\
uobject/uobject.h

BUILT_SOURCES = uobject/uobject.h
uobject/uobject.h: uobject/uobject.template.h
	cd uobject && $(MAKE) uobject.h

liburbi_la_SOURCES =				\
lockable.h                                      \
uabstractclient.h	uabstractclient.cpp	\
uclient.h		uclient.cpp		\
uconversion.h		uconversion.cpp		\
uobject/common_uvalue.cc			\
uobject/uexternal.h				\
uobject/uobject.h	uobject/uobject.cc	\
uobject/uvar.cc					\
usyncclient.h		usyncclient.cpp
liburbi_la_LIBADD = $(top_builddir)/jpeg-6b/libjpeg.la $(PTHREAD_LIBS)


## ------------ ##
## liburbi.la.  ##
## ------------ ##

env_LTLIBRARIES = libkernel-remote.la
libkernel_remote_la_SOURCES = $(liburbi_la_SOURCES) $(top_srcdir)/sdk/umain.cc
libkernel_remote_la_LIBADD = $(AM_LIBADD) $(top_builddir)/jpeg-6b/libjpegx.la
