## Copyright (C) 2006-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include $(top_srcdir)/build-aux/make/init.mk
include $(top_srcdir)/build-aux/make/libport.mk

# This is what modules' configure.ac defines as the variable that
# points to the install headers.  We use a non-installed kernel.
#
# FIXME: When mature, move into urbi-with-urbi-sdk so that we can
# point to a kernel build-tree instead of an installed kernel.
URBI_SDK_CPPFLAGS =				\
  $(LIBPORT_CPPFLAGS)				\
  $(BOOST_CPPFLAGS)				\
  -I$(kernel_srcdir)/include			\
  -I$(sdk_remote_srcdir)/include		\
  -I$(sdk_remote_builddir)/include

# Needed to compile UObjects on Windows.
sdk_remote_builddir = $(top_builddir)/sdk-remote
LIBUOBJECT_REMOTE_LIBS =						\
  $(sdk_remote_builddir)/src/libuobject/libuobject$(LIBSFX).la

module_LTLIBRARIES =
moduledir = $(urbi_uobjectsdir)
include modules.mk
