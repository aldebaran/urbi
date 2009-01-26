## -------------------------- ##
## liburbi-check test suite.  ##
## -------------------------- ##

# From a test file to a log file.
# Do not use a regular `.test.log:' rule here, since in that case the
# following rule (without incoming extension) will mask this one.
%.log: %.cc $(check_programs)
	@$(am__check_pre) bin/liburbi-check $(srcdir)/$* $(am__check_post)

# Find libport's sources and config.h.
AM_CPPFLAGS += $(LIBPORT_CPPFLAGS)
AM_CPPFLAGS += $(BOOST_CPPFLAGS)

AM_LDADD =					\
  $(top_builddir)/src/liburbi/liburbi.la	\
  $(top_builddir)/jpeg-6b/libjpeg.la		\
  $(PTHREAD_LIBS)

# tests(*).
check_PROGRAMS = bin/tests

bin_tests_SOURCES = bin/tests.hh bin/tests.cc $(LIBURBI_TESTS)
bin_tests_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)
bin_tests_CXXFLAGS = $(AM_CXXFLAGS) -Wno-unused-parameter
nodist_bin_tests_SOURCES = bin/dispatcher.cc
CLEANFILES += $(nodist_bin_tests_SOURCES)

bin/dispatcher.cc: $(tests_SOURCES) Makefile
	{								     \
	  echo '#include "bin/tests.hh"';				     \
	  echo;								     \
	  echo 'void';							     \
	  echo 'dispatch(const std::string& method, urbi::UClient& client,'; \
	  echo '	 urbi::USyncClient& syncClient)';		     \
	  echo '{';							     \
	  for f in $(LIBURBI_TESTS); do					     \
	    name=$$(echo $$f | sed -e 's@.*/@@;s@\..*@@');		     \
	    echo "  TESTS_RUN($$name);";				     \
	  done;								     \
	  echo '  std::cerr << "Unknown test " << method << std::endl;';     \
	  echo '  exit(1);';						     \
	  echo '}';							     \
	} >$@.tmp
	mv $@.tmp $@

# liburbi-check
EXTRA_DIST += bin/liburbi-check.m4sh
nodist_check_SCRIPTS += bin/liburbi-check
m4sh_scripts += bin/liburbi-check

# The test suite.
LIBURBI_TESTS =					\
  liburbi/values.cc				\
  liburbi/removecallbacks.cc


TESTS += $(LIBURBI_TESTS)
