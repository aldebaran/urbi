include $(top_srcdir)/flags.mk

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

SUBDIRS =
if BINDINGS
 SUBDIRS += swig
endif
SUBDIRS += . tests

## --------- ##
## uobject.  ##
## --------- ##

include $(top_srcdir)/build-aux/svn-externals.mk
include uobject/uobject.mk
urbiinclude_HEADERS = $(uobject_headers)

## ------------ ##
## liburbi.la.  ##
## ------------ ##

lib_LTLIBRARIES = liburbi.la

liburbi_la_SOURCES =				\
package-info.cc					\
$(uobject_sources)				\
uabstractclient.cc				\
ucallbacks.cc					\
uclient.cc					\
uconversion.cc					\
uobject.cc					\
uvar.cc						\
usystem.cc                                      \
usyncclient.cc
liburbi_la_LIBADD = $(LIBADD)


## --------------------- ##
## libkernel-remove.la.  ##
## --------------------- ##

env_LTLIBRARIES = libkernel-remote.la
libkernel_remote_la_SOURCES = $(liburbi_la_SOURCES) umain.cc
libkernel_remote_la_LIBADD = $(LIBADD)
