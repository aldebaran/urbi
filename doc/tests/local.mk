## ------- ##
## Check.  ##
## ------- ##

TESTS =

# The tests are generated in the src tree, but since it changes often,
# we don't put them in the repository.  That's no reason to forget to
# ship it.
local_mks = $(patsubst %.tex,$(srcdir)/tests/%/local.mk,$(call ls_files,*.tex))
-include $(local_mks)

EXTRA_DIST +=					\
  $(local_mks)					\
  $(TESTS)

include $(top_srcdir)/build-aux/check.mk

TEST_LOGS = $(TESTS:.chk=.log)
LAZY_TEST_SUITE = 1
$(TEST_LOGS): $(top_builddir)/all.stamp

# Generating the test files.
$(srcdir)/tests/%/local.mk: %.tex $(srcdir)/tex2chk
	srcdir=$(srcdir) $(srcdir)/tex2chk $<


URBI_CONSOLE = $(top_builddir)/tests/bin/urbi-console

# Set URBI_PATH to find URBI.INI.
TESTS_ENVIRONMENT +=				\
  URBI_CONSOLE=$(URBI_CONSOLE)			\
  URBI_PATH=$(abs_srcdir)/specs:$$URBI_PATH	\
  srcdir=$(srcdir)/tests\
  PATH=$(abs_top_builddir)/tests/bin:$$PATH

UCONSOLE_CHECK = $(top_builddir)/tests/bin/uconsole-check
UCONSOLE_CHECKFLAGS = -k2 --no-locations

%.log: %.chk
	@$(am__check_pre) $(UCONSOLE_CHECK) $(UCONSOLE_CHECKFLAGS) $${dir}$< $(am__check_post)

debug:
	echo $(TESTS)
	echo $(TEST_LOGS)


# The target run by the buildfarm.
.PHONY: check-buildfarm
CHECK_BUILDFARM_FLAGS = AM_COLOR_TESTS=no VERBOSE=1 INSTRUMENT=1
check-buildfarm:
	$(MAKE) $(AM_MAKEFLAGS) check-html $(CHECK_BUILDFARM_FLAGS)
