## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = liburbi/version.hh
include $(top_srcdir)/build-aux/revision.mk

nodist_env_DATA = liburbi/version.hh

## ------------ ##
## liburbi.la.  ##
## ------------ ##

lib_LTLIBRARIES += liburbi/liburbi.la
liburbi_liburbi_la_SOURCES =			\
  libuco/uvalue-common.cc			\
  liburbi/package-info.cc			\
  liburbi/uabstractclient.cc			\
  liburbi/uclient.cc				\
  liburbi/uconversion.cc			\
  liburbi/umessage.cc				\
  liburbi/usyncclient.cc

liburbi_liburbi_la_CPPFLAGS =			\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK
# Find version.hh.
liburbi_liburbi_la_CPPFLAGS += -I.

liburbi_liburbi_la_LIBADD = $(LIBADD) $(top_builddir)/jpeg-6b/libjpeg.la
liburbi_liburbi_la_LDFLAGS = -avoid-version -no-undefined


## ---------- ##
## umain.cc.  ##
## ---------- ##

dist_umain_DATA = liburbi/umain.cc
