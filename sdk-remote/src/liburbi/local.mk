# Path to envdir from top/install_prefix
topenvdir = $(PACKAGE_BRAND)/core/$(URBI_HOST)/$(URBI_ENV)

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

umaindir = $(datadir)/umain
dist_umain_DATA = liburbi/umain.cc

## ------------ ##
## liburbi.la.  ##
## ------------ ##

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

all-local: liburbi/liburbi.la.stamp

liburbi/liburbi.la.stamp: liburbi/liburbi.la
	$(top_srcdir)/build-aux/fix-libtool-la liburbi/liburbi.la \
	  liburbi/liburbi.la.stamp

CLEANFILES += liburbi/liburbi.la.stamp
