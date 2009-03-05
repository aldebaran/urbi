## ------- ##
## Check.  ##
## ------- ##

TESTS =
TEST_SUITE_LOG = tests/test-suite.log

# The tests are generated in the src tree, but since it changes often,
# we don't put them in the repository.  That's no reason to forget to
# ship it.
#
# We don't use '*.tex' blindly, as there are some in document-aux.
local_mks =							\
  $(patsubst %.tex,$(srcdir)/tests/%/local.mk,			\
             $(call ls_files,specs/*.tex tutorial/*.tex))
-include $(local_mks)

EXTRA_DIST +=					\
  $(local_mks)					\
  $(TESTS)					\
  tests/test.u

# Generating the test files.
$(srcdir)/tests/%/local.mk: %.tex $(srcdir)/tex2chk
	srcdir=$(srcdir) move_if_change=$(move_if_change) $(srcdir)/tex2chk $<


include $(top_srcdir)/build-aux/check.mk

TEST_LOGS = $(TESTS:.chk=.log)
LAZY_TEST_SUITE = 1
STRICT_TEST_LOGS =							   \
  $(shell $(AWK) '(FNR == 1 && $$1 == "FAIL:" ) { print $$2 }' $(TEST_LOGS))
$(TEST_LOGS): $(top_builddir)/all.stamp


URBI_CONSOLE = $(top_builddir)/tests/bin/urbi-console

# Set URBI_PATH to find URBI.INI.
TESTS_ENVIRONMENT +=				\
  URBI_CONSOLE=$(URBI_CONSOLE)			\
  URBI_PATH=$(abs_srcdir)/specs:$$URBI_PATH	\
  srcdir=$(srcdir)				\
  PATH=$(abs_top_builddir)/tests/bin:$$PATH

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
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENT=1
check-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) check-html $(CHECK_BUILDFARM_FLAGS)
