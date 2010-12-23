## Copyright (C) 2009-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## ------------- ##
## Urbi-launch.  ##
## ------------- ##

bin_PROGRAMS += bin/urbi-launch
# Find our own ltdl.h.  Push "system" headers last so that our headers
# have priority.
bin_urbi_launch_CPPFLAGS = -DURBI_ROOT_NOT_DLL $(LTDL_CPPFLAGS) $(AM_CPPFLAGS)
bin_urbi_launch_SOURCES  = bin/urbi-launch.cc liburbi/urbi-root.cc
bin_urbi_launch_LDFLAGS  = $(AM_LDFLAGS)
bin_urbi_launch_LDADD    = $(LTDL_LIBS)

if STATIC_BUILD
bin_urbi_launch_LDADD += $(sdk_remote_builddir)/src/libuvalue/libuvalue.la \
  $(sdk_remote_builddir)/src/libuobject/libuobject.la
endif
