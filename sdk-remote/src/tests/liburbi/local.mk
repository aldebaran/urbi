## Copyright (C) 2008-2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## -------------------------- ##
## liburbi-check test suite.  ##
## -------------------------- ##

# ---------------- #
# The test suite.  #
# ---------------- #
LIBURBI_TESTS =					\
  liburbi/0-empty.cc				\
  liburbi/ping.cc				\
  liburbi/removecallbacks.cc			\
  liburbi/syncvalues.cc				\
  liburbi/values.cc				\
  liburbi/xfail.cc

# This is on purpose, see xfail.cc.
XFAIL_TESTS += liburbi/xfail$(EXEEXT)

TESTS += $(LIBURBI_TESTS)

# ------------------------- #
# Compiling LIBURBI_TESTS.  #
# ------------------------- #

EXTRA_PROGRAMS = $(LIBURBI_TESTS:.cc=)
CLEANFILES += $(EXTRA_PROGRAMS)

# Find libport's sources and config.h.
AM_CPPFLAGS += $(LIBPORT_CPPFLAGS)
# Find bin/tests.hh.
AM_CPPFLAGS += -I$(srcdir)
# Find urbi/ headers.
AM_CPPFLAGS += -I$(sdk_remote_srcdir)/include
AM_CPPFLAGS += $(BOOST_CPPFLAGS)

AM_LDADD =							\
  $(sdk_remote_builddir)/src/liburbi/liburbi$(LIBSFX).la	\
  $(sdk_remote_builddir)/jpeg/libjpeg4urbi$(LIBSFX).la		\
  $(PTHREAD_LIBS)
# We should not need to report the -rpath to Boost here, since we
# don't depend directly from it (it is liburbi.la which does, via
# libport).  Yet Libtool does not propagate the rpath, so we need it
# here.
AM_LDFLAGS +=					\
  $(BOOST_THREAD_LDFLAGS)			\
  $(PTHREAD_LDFLAGS)

liburbi_0_empty_SOURCES         = bin/tests.hh bin/tests.cc liburbi/0-empty.cc
liburbi_ping_SOURCES            = bin/tests.hh bin/tests.cc liburbi/ping.cc
liburbi_removecallbacks_SOURCES = bin/tests.hh bin/tests.cc liburbi/removecallbacks.cc
liburbi_syncvalues_SOURCES      = bin/tests.hh bin/tests.cc liburbi/syncvalues.cc
liburbi_values_SOURCES          = bin/tests.hh bin/tests.cc liburbi/values.cc
liburbi_xfail_SOURCES           = bin/tests.hh bin/tests.cc liburbi/xfail.cc

# --------------- #
# liburbi-check.  #
# --------------- #

EXTRA_DIST += bin/liburbi-check.m4sh
M4SHFLAGS += -I$(srcdir)/m4sh
nodist_check_SCRIPTS += bin/liburbi-check
m4sh_scripts += bin/liburbi-check

# From a test file to a log file.
# Do not use a regular `.test.log:' rule here, since in that case the
# following rule (without incoming extension) will mask this one.
$(LIBURBI_TESTS:.cc=.log): %.log: %$(EXEEXT) $(check_programs) ../libraries.stamp
	@$(am__check_pre) bin/liburbi-check $(srcdir)/$* $(am__check_post)
