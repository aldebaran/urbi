include $(top_srcdir)/flags.mk

# Path to envdir from top/install_prefix
topenvdir=$(PACKAGE_BRAND)/core/$(URBI_HOST)/$(URBI_ENV)

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

SUBDIRS =
if BINDINGS
 SUBDIRS += swig
endif
SUBDIRS += . #tests

## --------- ##
## uobject.  ##
## --------- ##

include $(top_srcdir)/build-aux/svn-externals.mk
uobject_srcdir = $(srcdir)/uobject
include uobject/uobject.mk
urbiinclude_HEADERS = $(uobject_headers)

## ------------ ##
## liburbi.la.  ##
## ------------ ##

umaindir=$(datadir)/umain
umain_DATA = umain.cc

env_LTLIBRARIES = libuobject.la
lib_LTLIBRARIES = liburbi.la

libuobject_la_SOURCES =                         \
$(uobject_sources)				\
package-info.cc					\
ucallbacks.cc					\
uobject.cc					\
usystem.cc					\
utimer-callback.cc				\
uvar.cc

liburbi_la_SOURCES =				\
uobject/uvalue-common.cc                        \
uabstractclient.cc				\
uclient.cc					\
uconversion.cc					\
usyncclient.cc


liburbi_la_CPPFLAGS = $(AM_CPPFLAGS) -DBUILDING_URBI_SDK -DBUILDING_LIBPORT
liburbi_la_LIBADD = $(LIBADD)
liburbi_la_LDFLAGS = -no-undefined

libuobject_la_CPPFLAGS = $(AM_CPPFLAGS) -DBUILDING_URBI_SDK -DBUILDING_LIBPORT
libuobject_la_LIBADD = $(LIBADD) liburbi.la
libuobject_la_LDFLAGS = -no-undefined



