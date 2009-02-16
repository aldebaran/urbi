## ------- ##
## Check.  ##
## ------- ##

TESTS =
# We don't ship them: they are generated when the doc test suite is.
-include $(call ls_files,tests/*/*/local.mk)
include $(top_srcdir)/build-aux/check.mk

TEST_LOGS = $(TESTS:.chk=.log)
# FIXME: Assigned to QH.
TFAIL_TESTS += specs/lang.tex

# Generating the test files.
noinst_DATA = $(patsubst %.tex,$(srcdir)/tests/%/local.mk,$(call ls_files,*.tex))
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