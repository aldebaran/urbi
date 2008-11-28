include $(top_srcdir)/build-aux/init.mk
AUTOMAKE_OPTIONS += subdir-objects
include $(top_srcdir)/build-aux/check.mk
include $(top_srcdir)/build-aux/m4sh.mk

## ---------------------------------------- ##
## Wrappers for non-installed executables.  ##
## ---------------------------------------- ##

configured_scripts =				\
  bin/umake					\
  bin/umake-engine				\
  bin/umake-lib					\
  bin/umake-link				\
  bin/umake-remote				\
  bin/umake-shared				\
  bin/urbi-launch

nodist_noinst_SCRIPTS += $(configured_scripts)

# This is really painful: dependencies on config.status are handled
# only in the top-level Makefile by Automake.  So enforce these
# changes here.  Which basically amounts to duplicating what Automake
# does in the top-level Makefile.
$(configured_scripts): bin/%: $(wildcard $(srcdir)/bin/*.in)
	cd $(top_builddir) && $(SHELL) ./config.status src/tests/$@



## -------------------- ##
## uobject test suite.  ##
## -------------------- ##

# Compile each UObject using umake.

# I couldn't get git-ls-files to list directories.  So ask for the CC
# files in them, and keep only the *.uob part.
UOBJECTS = $(patsubst %/,%,$(sort $(dir $(call ls_files,uobjects/*.uob/*.cc))))

# uobject-check
EXTRA_DIST += bin/uobject-check.as
nodist_check_SCRIPTS += bin/uobject-check
m4sh_scripts += bin/uobject-check


%.log: %.uob bin/uobject-check
	@$(am__check_pre) bin/uobject-check $(srcdir)/$* $(am__check_post)

debug:
	echo $(UOBJECTS)
	echo $(TEST_LOGS)

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

# By default, instrument the code.
INSTRUMENT = true
URBI_SERVER = urbi-console

TESTS_ENVIRONMENT =				\
  srcdir='$(srcdir)'				\
  INSTRUMENT=$(INSTRUMENT)			\
  PACKAGE_NAME='$(PACKAGE_TARNAME)'		\
  URBI_SERVER=$(URBI_SERVER)			\
  PATH=$(abs_builddir)/bin:$$PATH

# tests(*).
check_PROGRAMS = bin/tests

bin_tests_SOURCES = bin/tests.hh bin/tests.cc $(CHECKFILES)
bin_tests_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)
bin_tests_CXXFLAGS = $(AM_CXXFLAGS) -Wno-unused-parameter
nodist_bin_tests_SOURCES = bin/dispatcher.cc
CLEANFILES += $(nodist_tests_SOURCES)

bin/dispatcher.cc: $(tests_SOURCES) Makefile
	{								     \
	  echo '#include "bin/tests.hh"';				     \
	  echo;								     \
	  echo 'void';							     \
	  echo 'dispatch(const std::string& method, urbi::UClient& client,'; \
	  echo '	 urbi::USyncClient& syncClient)';		     \
	  echo '{';							     \
	  for f in $(CHECKFILES); do					     \
	    name=$$(echo $$f | sed -e 's@.*/@@;s@\..*@@');		     \
	    echo "  TESTS_RUN($$name);";				     \
	  done;								     \
	  echo '  std::cerr << "Unknown test " << method << std::endl;';     \
	  echo '  exit(1);';						     \
	  echo '}';							     \
	} >$@.tmp
	mv $@.tmp $@

# liburbi-check
EXTRA_DIST += bin/liburbi-check.as
nodist_check_SCRIPTS += bin/liburbi-check
m4sh_scripts += bin/liburbi-check

# The test suite.
CHECKFILES = 					\
  checkfiles/values.cc				\
  checkfiles/removecallbacks.cc
TESTS = $(CHECKFILES) $(UOBJECTS)
TEST_LOGS = $(addsuffix .log,$(basename $(TESTS)))

# Run some tests only.
check-%:
        $(MAKE) $(AM_MAKEFLAGS) check-html TESTS_DIRS=$* VERBOSE=1

check-clean-local:
	rm -rf $(CHECKFILES:.cc=.dir)
