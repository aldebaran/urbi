include $(top_srcdir)/build-aux/init.mk

SUBDIRS =
if BINDINGS
 SUBDIRS += swig
endif
SUBDIRS += .

# Find libjpeg's src (jpeglib.h) and build (jconfig.h) files.
AM_CPPFLAGS += -I$(top_srcdir) -I$(top_builddir)/jpeg-6b
# Find urbi/ headers.
AM_CPPFLAGS += -I$(top_srcdir)/include

AM_CXXFLAGS += $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)
AM_LIBADD   += $(PTHREAD_LIBS)


## --------- ##
## uobject.  ##
## --------- ##

include uobject/uobject.mk

sdkincludeuobjectdir = $(sdkincludedir)/uobject
sdkincludeuobject_HEADERS = 			\
uobject/uobject.hh

## ------------ ##
## liburbi.la.  ##
## ------------ ##

lib_LTLIBRARIES = liburbi.la

liburbi_la_SOURCES =				\
$(uobject_sources)				\
uabstractclient.cc				\
uclient.cc					\
uconversion.cc					\
uobject.cc					\
uvar.cc						\
usyncclient.cc
liburbi_la_LIBADD = $(top_builddir)/jpeg-6b/libjpeg.la $(PTHREAD_LIBS)


## --------------------- ##
## libkernel-remove.la.  ##
## --------------------- ##

env_LTLIBRARIES = libkernel-remote.la
libkernel_remote_la_SOURCES = $(liburbi_la_SOURCES) $(top_srcdir)/sdk/umain.cc
libkernel_remote_la_LIBADD = $(AM_LIBADD) $(top_builddir)/jpeg-6b/libjpeg.la
