## Copyright (C) 2005-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include $(top_srcdir)/build-aux/init.mk
include $(top_srcdir)/build-aux/fix-libtool-la.mk
include $(top_srcdir)/build-aux/libport.mk

AUTOMAKE_OPTIONS += subdir-objects

SUBDIRS = . tests
if BINDINGS
 SUBDIRS += swig
endif

# libjpeg.
JPEG_CPPFLAGS = -I$(top_srcdir)/jpeg -I$(top_builddir)/jpeg
JPEG_LIBS = $(top_builddir)/jpeg/libjpeg4urbi$(LIBSFX).la

AM_CPPFLAGS +=					\
  $(LIBPORT_CPPFLAGS)				\
  $(JPEG_CPPFLAGS)				\
  -I$(top_srcdir)/include -I$(top_builddir)	\
  -I$(srcdir)

AM_CXXFLAGS += $(PTHREAD_CFLAGS) $(SOCKETS_CFLAGS) $(WARNING_CXXFLAGS)

LIBADD =					\
  $(PTHREAD_LIBS)				\
  $(SOCKETS_LIBS)				\
  $(LIBPORT_LIBS)

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

# To be linked in all the executables (urbi-launch, examples/ etc.).
AM_LDADD =					\
  liburbi/liburbi$(LIBSFX).la			\
  $(JPEG_LIBS)					\
  $(PTHREAD_LIBS)				\
  $(LIBPORT_LIBS)

# The order matters: build dependencies first.
include libuvalue/local.mk
include libuco/local.mk

.PHONY: installcheck-umake
if SDK_REMOTE
include bin/local.mk
include liburbi/local.mk

if !STATIC_BUILD
include urbi/local.mk
endif !STATIC_BUILD

include libuobject/local.mk
endif SDK_REMOTE

if EXAMPLES
include examples/local.mk
include utils/local.mk
endif EXAMPLES


## ---------------------- ##
## Convenient shortcuts.  ##
## ---------------------- ##

.PHONY: core libs
libs: $(LTLIBRARIES)
core: libs bin/urbi-launch$(EXEEXT)



## --------- ##
## Install.  ##
## --------- ##
install-exec-hook: post-install
post-install:
# If we are used as a module of Urbi SDK, there is no need to run this
# post-install step here, Urbi SDK.  Besides, under some
# circumstances, running it twice can result in failure.
	if test -n "$(DESTDIR)" &&				\
	   ! grep 'PACKAGE_TARNAME = urbi-sdk'			\
	       $(top_builddir)/../Makefile >/dev/null 2>&1;	\
	then							\
	  $(build_aux_dir)/bin/check-lib-deps			\
	    --verbose --verbose --verbose --verbose		\
	    --destdir "$(DESTDIR)"				\
	    --host-os "$(URBI_HOST_OS)"				\
	    --prefix "$(prefix)"				\
	    --libdir "$(libdir)"				\
	    --libpath "$(BOOST_LDPATH)"				\
	    "$(DESTDIR)";					\
	fi

## ----------- ##
## Buildfarm.  ##
## ----------- ##

# The target run by the buildfarm.
CHECK_BUILDFARM_FLAGS = -j4 AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENTATION=0.2
%check-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) $(CHECK_BUILDFARM_FLAGS) $*check-html
