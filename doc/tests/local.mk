# Be sure to run up-to-date checking binaries.
check-TESTS: tests-check-programs
tests-check-programs:
	$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir)/tests am-check-programs

## ------- ##
## Check.  ##
## ------- ##

TESTS =
TEST_SUITE_LOG = tests/test-suite.log

# The tests are generated in the src tree, but since it changes often,
# we don't put them in the repository.  That's no reason to forget to
# ship it.
test_mks =					\
  $(patsubst %.tex,$(srcdir)/tests/%/test.mk,	\
    $(filter-out document-aux/%,		\
      $(call ls_files,*.tex)))
-include $(test_mks)

EXTRA_DIST +=					\
  $(test_mks)					\
  $(TESTS)					\
  tests/test.u

# Generating the test files.
$(srcdir)/tests/%/test.mk: %.tex $(srcdir)/tex2chk
	srcdir=$(srcdir) move_if_change=$(move_if_change) $(srcdir)/tex2chk $<


include $(top_srcdir)/build-aux/check.mk

# A target that just runs "make check", we don't want to "make all" as
# is the default with "make check-am".
just-check: $(nodist_check_SCRIPTS)
	$(MAKE) $(AM_MAKEFLAGS) check-TESTS


TEST_LOGS = $(TESTS:.chk=.log)
LAZY_TEST_SUITE = 1
STRICT_TEST_LOGS = $(shell $(LIST_FAILED_TEST_LOGS))
$(TEST_LOGS): $(top_builddir)/all.stamp


# Set URBI_PATH to find URBI.INI.
TESTS_ENVIRONMENT +=				\
  URBI_PATH=$(abs_srcdir)/specs:$$URBI_PATH	\
  srcdir=$(srcdir)				\
  PATH=$(abs_top_builddir)/sdk-remote/src/tests/bin:$$PATH

UCONSOLE_CHECK = $(top_builddir)/tests/bin/uconsole-check
UCONSOLE_CHECKFLAGS = -k2 --no-locations

%.log: %.chk
	@$(am__check_pre) $(UCONSOLE_CHECK) $(UCONSOLE_CHECKFLAGS) $${dir}$< $(am__check_post)
check-clean-local:
	-chmod -R 700 $(TEST_LOGS:.log=.dir) 2>/dev/null
	rm -rf $(TEST_LOGS:.log=.dir)

## ----------- ##
## Buildfarm.  ##
## ----------- ##

.PHONY: check-buildfarm
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 # INSTRUMENTATION=1
check-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) check-html $(CHECK_BUILDFARM_FLAGS)

.PHONY check-TESTS: check-clean-semaphores
check-clean-semaphores:
	-$(build_aux_dir)/semaphore-clean.sh
