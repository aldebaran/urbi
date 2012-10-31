## Copyright (C) 2006-2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include $(top_srcdir)/build-aux/make/init.mk
include $(top_srcdir)/build-aux/make/m4sh.mk
M4SHFLAGS += -I $(srcdir)

## ---------------------------------------- ##
## Wrappers for non-installed executables.  ##
## ---------------------------------------- ##

# Make check in SDK-Remote wants to use this scripts.  And we go there
# first.  So make this a noinst_SCRIPT, not a check_SCRIPT.
nodist_noinst_SCRIPTS +=				\
  bin/urbi$(EXEEXT)

# This is really painful: dependencies on config.status are handled
# only in the top-level Makefile by Automake.  So enforce these
# changes here.  Which basically amounts to duplicating what Automake
# does in the top-level Makefile.
bin/%: bin/%.in
	cd $(top_builddir) && $(SHELL) ./config.status tests/$@

bin/urbi$(EXEEXT): bin/urbi.in
	cd $(top_builddir) && $(SHELL) ./config.status tests/$@

## ------------------ ##
## uconsole wrapper.  ##
## ------------------ ##

m4sh_deps +=  bin/kernel-check.m4sh
EXTRA_DIST += bin/uconsole.m4sh
nodist_check_SCRIPTS += bin/uconsole
m4sh_scripts += bin/uconsole


## ---------------------- ##
## uconsole tests suite.  ##
## ---------------------- ##

edit +=							\
  -e 's|@DISABLED_MODULES[@]|$(DISABLED_MODULES)|g'	\
  -e 's|@ENABLED_MODULES[@]|$(ENABLED_MODULES)|g'	\
  -e 's|@ROS_ROOT[@]|$(ROS_ROOT)|g'			\
  -e 's|@ROS_SETUP[@]|$(ROS_SETUP)|g'			\
  -e 's|@URBI_SERVER[@]|$(URBI_SERVER)|g'

# Our tests, globbing accepted.
TESTS = $(addsuffix /*.chk,$(TESTS_DIRS))

# Globbing resolved.
tests = $(call ls_files,$(TESTS))

# Before running the test suite, check that the dirs exist.
.PHONY check-TESTS: check-dirs
check-dirs: clean-semaphores
	@for i in $(TESTS_DIRS);				\
        do							\
	  if test ! -d $(srcdir)/$i; then			\
            echo >&2 "TESTS_DIRS: $$i is not a directory";	\
	    exit=false;						\
	 fi;							\
        done;							\
	$$exit

# Check that XFAIL tests do exist.
check-test-list-%:
	@exit=true;						\
	for f in $($*); do					\
	  if test ! -f $$f && test ! -f $(srcdir)/$$f; then	\
	    echo >&2 "$*: $$f does not exist";			\
	    exit=false;						\
	  fi;							\
	done;							\
	$$exit

.PHONY check-TESTS: check-tests
check-tests: check-test-list-XFAIL_TESTS check-test-list-TFAIL_TESTS

# Run some tests only.
check-k1: check-1.x
check-%:
	$(MAKE) $(AM_MAKEFLAGS) check-html TESTS_DIRS=$* VERBOSE=1

TEST_LOGS = $(addsuffix .log,$(basename $(tests)))
# Rerun only what failed, since sometimes it's only because of CPU
# overloads that touchy tests failed.
LAZY_TEST_SUITE ?= 1
STRICT_TEST_LOGS = $(shell $(LIST_FAILED_TEST_LOGS))
$(TEST_LOGS): $(top_builddir)/all.stamp

# The Java tests depend on the Java timestamp.
JAVA_LOGS =						\
  $(strip						\
    $(foreach log,$(TEST_LOGS),				\
       $(if $(findstring /java/,'$(log)'),$(log))))
$(JAVA_LOGS): $(top_builddir)/java.stamp

# Parallel test framework.
include $(top_srcdir)/build-aux/make/check.mk

# Ship all the ressources needed.
EXTRA_DIST += $(call ls_files,*.chk *.u)

debug:
	@echo 'TESTS_DIRS=$(TESTS_DIRS)'
	@echo 'TESTS=$(TESTS)'
	@echo 'tests=$(tests)'
	@echo 'EXTRA_DIST=$(EXTRA_DIST)'
	@echo 'STRICT_TEST_LOGS=$(STRICT_TEST_LOGS)'
	@echo 'JAVA_LOGS=$(JAVA_LOGS)'

# Failing on purpose, to exercize error handling.
XFAIL_TESTS +=					\
  0.x/2-xfail.chk				\
  uob/java/xfail.chk

## ---------------- ##
## uconsole-check.  ##
## ---------------- ##

EXTRA_DIST += bin/uconsole-check.m4sh bin/neutralize
nodist_check_SCRIPTS += bin/uconsole-check
m4sh_scripts += $(UCONSOLE_CHECK)

# From a test file to a log file.
UCONSOLE_CHECK = bin/uconsole-check
UCONSOLE_CHECKFLAGS =
if COMPILATION_MODE_SPACE
  UCONSOLE_CHECKFLAGS += --no-error-verbose
endif

# Not all the test depend on all the *.u, but it is safer to be too
# touchy than not enough.  Be my guest to improve.
test_deps :=  $(call ls_files,*.u) bin/neutralize
%.log: %.chk $(check_programs) $(test_deps)
	@$(am__check_pre)					\
	  $(UCONSOLE_CHECK) $(UCONSOLE_CHECKFLAGS) $${dir}$<	\
	$(am__check_post)
check-clean-local:
	-chmod -R 700 $(TEST_LOGS:.log=.dir) 2>/dev/null
	rm -rf $(TEST_LOGS:.log=.dir)

## ------------- ##
## Bench suite.  ##
## ------------- ##

BENCHES_DIRS = benches
BENCH_LOGFLAGS = --test-suite
BENCH_LOGS = $(TEST_LOGS)
AM_BENCHFLAGS = --hook-module=$(BENCH_MALLOC_HOOK) --output-once --format=xls
include $(top_srcdir)/build-aux/make/bench.mk

bench: $(BENCH_MALLOC_HOOK)
	$(MAKE) $(AM_MAKEFLAGS)				\
	  LAZY_TEST_SUITE=$(LAZY_BENCH_SUITE)		\
	  TESTS_DIRS='$(BENCHES_DIRS)'			\
	  BENCHFLAGS='$(AM_BENCHFLAGS) $(BENCHFLAGS)'	\
	  BENCH=1 check
	$(MAKE) $(AM_MAKEFLAGS)				\
	  TESTS_DIRS='$(BENCHES_DIRS)'			\
	  BENCHFLAGS='$(AM_BENCHFLAGS) $(BENCHFLAGS)'	\
	  $(BENCH_LOG_FILE)

## ------------------- ##
## Tests environment.  ##
## ------------------- ##

# By default, tests are buildcheck.
TESTS_ENVIRONMENT += $(BUILDCHECK_ENVIRONMENT)

# Use the wrappers to run the non-installed executables.
BUILDCHECK_ENVIRONMENT +=					\
  PATH=$(sdk_remote_builddir)/src/tests/bin:$$PATH

INSTALLCHECK_ENVIRONMENT +=					\
  PATH=$(DESTDIR)$(bindir):$$PATH

# Run the tests with the install-environment.
installcheck-local:
	$(MAKE) $(AM_MAKEFLAGS)					\
	  TESTS_ENVIRONMENT='$$(INSTALLCHECK_ENVIRONMENT)' check

# Local changes that depend upon the hosting project.
#
# This file must define TESTS_DIR, extend XFAIL_TESTS, etc.
#
# Define URBI_SERVER to the name of the server to use.  The kernel
# builds "urbi" so it should set URBI_SERVER to "urbi".  The engine
# usually rather build "urbi-server" programs, in which case define
# URBI_SERVER to urbi-server.  And so forth.
include $(top_srcdir)/tests-local.mk
