## Copyright (C) 2005-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include $(top_srcdir)/build-aux/make/init.mk
include $(top_srcdir)/build-aux/make/fix-libtool-la.mk
include $(top_srcdir)/build-aux/make/build-farm.mk

# The doc/ documentation (which is part of .) needs to process some
# files that are generated in src/ (e.g., urbi.java for
# sdk-remote-java's documentation).
SUBDIRS =					\
  jpeg						\
  libport					\
  src						\
  .

# Also fix libjpeg.la.
LTLIBRARIES_TO_FIX += jpeg/libjpeg4urbi$(LIBSFX).la

brandlibexec_SCRIPTS =
AM_CPPFLAGS +=					\
  $(LIBPORT_CPPFLAGS)				\
  -I$(srcdir)/lib				\
  $(BOOST_CPPFLAGS)

include sdk/local.mk
include include/local.mk
include doc/local.mk


## ------- ##
## Check.  ##
## ------- ##

# check-html, check-clean, check-uob etc.
check-%:
	$(MAKE) $(AM_MAKEFLAGS) -C src/tests $@
recheck recheck-html: clean-semaphores
	$(MAKE) $(AM_MAKEFLAGS) -C src $@
	$(MAKE) $(AM_MAKEFLAGS) -C src/tests $@
