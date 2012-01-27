## Copyright (C) 2009-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

# Be sure to run up-to-date checking binaries.
check-TESTS: tests-check-programs
tests-check-programs:
	$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir)/tests am-check-programs
check-TESTS: clean-semaphores

## ------- ##
## Check.  ##
## ------- ##

TESTS =
TEST_SUITE_LOG = tests/test-suite.log

# The list of LaTeX source files from the Kernel documentation that
# will be turned into tests.
test_tex =					\
  $(filter-out					\
     $(FROM_GEN) document-aux/% tmp.t2d/%,	\
     $(call ls_files,*.tex))

# Look in addons/ if there are additional LaTeX files that should
# generate test cases.  To this end, we call `ls-file
# @{addons/foo}/*.tex for each foo in addons/.
addons = 					\
  $(patsubst $(srcdir)/%,%,			\
    $(wildcard $(srcdir)/addons/*))

test_tex +=					\
  $(if $(addons),				\
    $(call ls_files,				\
      $(patsubst %,@{%}/*.tex,			\
        $(addons))))

# From each LaTeX source a test.mk is generated.  E.g.,
# specs/string.tex => $(srcdir)/tests/specs/strings-test.mk.  This is
# the set of these *-test.mk files.
test_mks =					\
  $(patsubst %.tex,$(srcdir)/tests/%-test.mk,	\
     $(test_tex))

-include $(test_mks)

# The tests are generated in the src tree, but since it changes often,
# we don't put them in the repository.  That's no reason to forget to
# ship it.
EXTRA_DIST +=					\
  $(test_mks)					\
  $(TESTS)					\
  tests/test.u

# Generating the test files.
# We specify that it applies to $(test_mks) to take precedence over
# the default rule for %.mk in make/init.mk
EXTRA_DIST += bin/tex2chk
$(srcdir)/tests/%-test.mk: %.tex $(srcdir)/bin/tex2chk
	$(AM_V_GEN)					\
	  srcdir=$(srcdir)				\
	  move_if_change="$(move_if_change_run)"	\
	  $(srcdir)/bin/tex2chk $(if $(V:0=),-vv,-q) $<


include $(top_srcdir)/build-aux/make/check.mk

# A target that just runs "make check", we don't want to "make all" as
# is the default with "make check-am".
check-doc: $(nodist_check_SCRIPTS)
	$(MAKE) $(AM_MAKEFLAGS) check-TESTS
	$(MAKE) $(AM_MAKEFLAGS) spell-check

check-addons:
	$(MAKE) $(AM_MAKEFLAGS) check-doc \
	  TESTS="$(call ls_files, @{tests/addons}/*.chk)"

TEST_LOGS = $(TESTS:.chk=.log)
LAZY_TEST_SUITE = 1
STRICT_TEST_LOGS = $(shell $(LIST_FAILED_TEST_LOGS))
$(TEST_LOGS): $(top_builddir)/all.stamp


TESTS_ENVIRONMENT +=						\
  srcdir=$(srcdir)						\
  PATH=$(abs_top_builddir)/sdk-remote/src/tests/bin:$$PATH

UCONSOLE_CHECK = $(top_builddir)/tests/bin/uconsole-check
UCONSOLE_CHECKFLAGS = -k2 --no-locations

tests_deps := $(call ls_files, *.u)
%.log: %.chk $(tests_deps)
	@$(am__check_pre) $(UCONSOLE_CHECK) $(UCONSOLE_CHECKFLAGS) $${dir}$< $(am__check_post)
check-clean-local:
	-chmod -R 700 $(TEST_LOGS:.log=.dir) 2>/dev/null
	rm -rf $(TEST_LOGS:.log=.dir)


## ------------------- ##
## Tests environment.  ##
## ------------------- ##

# Our system leaves trailing unused *chk files.
MAINTAINERCLEANFILES +=				\
  $(shell find $(srcdir) -name '*.chk')

# Use the wrappers to run the non-installed executables.
CHECK_ENVIRONMENT +=				\
  srcdir=$(srcdir)

BUILDCHECK_ENVIRONMENT +=				\
  PATH=$(sdk_remote_builddir)/src/tests/bin:$$PATH

INSTALLCHECK_ENVIRONMENT +=			\
  PATH=$(DESTDIR)$(bindir):$$PATH

# By default, tests are buildcheck.
TESTS_ENVIRONMENT +=				\
  $(BUILDCHECK_ENVIRONMENT)

# Run the tests with the install-environment.
installcheck-local:
	$(MAKE) $(AM_MAKEFLAGS)						\
	  TESTS_ENVIRONMENT='$$(INSTALLCHECK_ENVIRONMENT)' check



## -------- ##
## XFAILS.  ##
## -------- ##

TFAIL_TESTS +=


include $(top_srcdir)/tests-local.mk
