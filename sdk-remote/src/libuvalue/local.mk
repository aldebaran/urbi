## Copyright (C) 2009-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## ------------ ##
## version.hh.  ##
## ------------ ##

REVISION_FILE = libuvalue/version.hh
REVISIONFLAGS = --stub
include $(top_srcdir)/build-aux/revision.mk

nodist_env_DATA = $(REVISION_FILE)

## --------------------- ##
## Sources and headers.  ##
## --------------------- ##

noinst_LTLIBRARIES += libuvalue/libuvalue.la
libuvalue_libuvalue_la_CPPFLAGS =		\
  $(AM_CPPFLAGS)				\
  -DBUILDING_URBI_SDK
# Find version.hh.
libuvalue_libuvalue_la_CPPFLAGS += -I.
libuvalue_libuvalue_la_LDFLAGS = $(BOOST_THREAD_LDFLAGS)
libuvalue_libuvalue_la_LIBADD = $(BOOST_THREAD_LIBS)

dist_libuvalue_libuvalue_la_SOURCES =		\
  liburbi/urbi-root.cc				\
  libuvalue/exit.cc				\
  libuvalue/package-info.cc			\
  libuvalue/ubinary.cc				\
  libuvalue/uimage.cc				\
  libuvalue/usound.cc				\
  libuvalue/uvalue-common.cc
