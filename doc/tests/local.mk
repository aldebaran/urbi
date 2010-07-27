# Be sure to run up-to-date checking binaries.
check-TESTS: tests-check-programs
tests-check-programs:
	$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir)/tests am-check-programs

## ------- ##
## Check.  ##
## ------- ##

TESTS =
TEST_SUITE_LOG = tests/test-suite.log

# The list of LaTeX source files from the Kernel documentation that
# will be turned into tests.
test_tex = 					\
  $(filter-out document-aux/%,			\
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
# specs/string.tex => $(srcdir)/tests/specs/strings/test.mk.  This is
# the set of these test.mk files.
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
$(srcdir)/tests/%-test.mk: %.tex $(srcdir)/bin/tex2chk
	srcdir=$(srcdir) move_if_change=$(move_if_change) \
	  $(srcdir)/bin/tex2chk $<


include $(top_srcdir)/build-aux/check.mk

# A target that just runs "make check", we don't want to "make all" as
# is the default with "make check-am".
just-check: $(nodist_check_SCRIPTS)
	$(MAKE) $(AM_MAKEFLAGS) check-TESTS
	$(MAKE) $(AM_MAKEFLAGS) spell-check


TEST_LOGS = $(TESTS:.chk=.log)
LAZY_TEST_SUITE = 1
STRICT_TEST_LOGS = $(shell $(LIST_FAILED_TEST_LOGS))
$(TEST_LOGS): $(top_builddir)/all.stamp


TESTS_ENVIRONMENT +=						\
  URBI_PATH=$$URBI_PATH						\
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

## ----------- ##
## Buildfarm.  ##
## ----------- ##

#.PHONY: check-buildfarm
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 # INSTRUMENTATION=1
%heck-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) $*heck-html $(CHECK_BUILDFARM_FLAGS)

.check-TESTS: clean-semaphores
