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
$(uobject_sources)				\
package-info.cc					\
uabstractclient.cc				\
ucallbacks.cc					\
uclient.cc					\
uconversion.cc					\
umain.cc					\
uobject.cc					\
usyncclient.cc					\
usystem.cc					\
uvar.cc

liburbi_la_LIBADD = $(LIBADD)
