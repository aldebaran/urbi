## Copyright (C) 2007-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include $(top_srcdir)/build-aux/init.mk
AUTOMAKE_OPTIONS += subdir-objects
TESTS =
AM_CXXFLAGS += $(WARNING_CXXFLAGS)
include $(top_srcdir)/build-aux/check.mk
include $(top_srcdir)/build-aux/m4sh.mk
include $(top_srcdir)/build-aux/libport.mk

include examples/local.mk
include liburbi/local.mk
include uobjects/local.mk

## ---------------------------------------- ##
## Wrappers for non-installed executables.  ##
## ---------------------------------------- ##

configured_scripts =				\
  bin/umake					\
  bin/umake-link				\
  bin/umake-shared				\
  bin/urbi-send

nodist_noinst_SCRIPTS +=                       \
  $(configured_scripts)                        \
  bin/urbi-launch$(EXEEXT)

# This is really painful: dependencies on config.status are handled
# only in the top-level Makefile by Automake.  So enforce these
# changes here.  Which basically amounts to duplicating what Automake
# does in the top-level Makefile.
$(configured_scripts): bin/%: $(wildcard $(srcdir)/bin/*.in)
	cd $(top_builddir) && $(SHELL) ./config.status src/tests/$@

bin/urbi-launch$(EXEEXT): bin/urbi-launch.in
	cd $(top_builddir) && $(SHELL) ./config.status --file src/tests/$@:src/tests/$<
	chmod +x $@

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
BUILDCHECK_PATH = $(abs_builddir)/bin:$(abs_top_builddir)/src/examples

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



## ----------- ##
## Buildfarm.  ##
## ----------- ##

# The target run by the buildfarm.
CHECK_BUILDFARM_FLAGS = -j2 AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENTATION=0.2
%check-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) $(CHECK_BUILDFARM_FLAGS) $*check-html

check-TESTS: clean-semaphores
