## Copyright (C) 2007-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include $(top_srcdir)/build-aux/make/init.mk
TESTS =
AM_CXXFLAGS += $(WARNING_CXXFLAGS)
# Rerun only what failed, since sometimes it's only because of CPU
# overloads that touchy tests failed.
LAZY_TEST_SUITE = 1
STRICT_TEST_LOGS = $(shell $(LIST_FAILED_TEST_LOGS))
include $(top_srcdir)/build-aux/make/check.mk
include $(top_srcdir)/build-aux/make/m4sh.mk
include $(top_srcdir)/build-aux/make/libport.mk

include examples/local.mk
include liburbi/local.mk
include uobjects/local.mk

## -------- ##
## Stamps.  ##
## -------- ##

# If stamps are missing, go and build them.  Be sure not to run
# several makes in parallel in the same directory.
stamps = all libraries executables
$(patsubst %,../%.stamp, $(stamps)):
	@set -x ; for i in $(stamps); do			\
	  if test ! -f ../$$i.stamp; then			\
	    $(MAKE) $(AM_MAKEFLAGS) -C .. $(stamps:=.stamp);	\
	    break;						\
	  fi;							\
	done

## ---------------------------------------- ##
## Wrappers for non-installed executables.  ##
## ---------------------------------------- ##

nodist_noinst_SCRIPTS +=                        \
  bin/umake                                     \
  bin/umake-java                                \
  bin/umake-link                                \
  bin/umake-shared                              \
  bin/urbi-launch$(EXEEXT)                      \
  bin/urbi-launch-java                          \
  bin/urbi-send

# This is really painful: dependencies on config.status are handled
# only in the top-level Makefile by Automake.  So enforce these
# changes here.  Which basically amounts to duplicating what Automake
# does in the top-level Makefile.
$(nodist_noinst_SCRIPTS): bin/%: $(wildcard $(srcdir)/bin/*.in)
	cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@

# By default, instrument the code.
#INSTRUMENTATION = true
URBI_SERVER = urbi-launch$(EXEEXT) --start --

## ------------------- ##
## Tests environment.  ##
## ------------------- ##

CHECK_ENVIRONMENT +=				\
  srcdir='$(srcdir)'				\
  INSTRUMENTATION=$(INSTRUMENTATION)		\
  PACKAGE_NAME='$(PACKAGE_TARNAME)'		\
  URBI_SERVER='$(URBI_SERVER)'

# Find (i) our wrappers, (ii) the examples programs.
BUILDCHECK_PATH =							\
  $(abs_builddir)/bin:$(abs_top_builddir)/sdk-remote/src/examples

# Use the wrappers to run the non-installed executables.
BUILDCHECK_ENVIRONMENT +=			\
  PATH=$(BUILDCHECK_PATH):$$PATH

INSTALLCHECK_ENVIRONMENT +=			\
  PATH=$(DESTDIR)$(bindir):$$PATH

# By default, tests are buildcheck.
TESTS_ENVIRONMENT += $(BUILDCHECK_ENVIRONMENT)

# Run the tests with the install-environment.
installcheck-local:
	$(MAKE) $(AM_MAKEFLAGS)					\
	  TESTS_ENVIRONMENT='$$(INSTALLCHECK_ENVIRONMENT)' check

TEST_LOGS = $(addsuffix .log,$(basename $(TESTS)))

check-clean-local:
	rm -rf $(TEST_LOGS:.log=.dir)

check-TESTS: clean-semaphores
