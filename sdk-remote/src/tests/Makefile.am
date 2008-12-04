include $(top_srcdir)/build-aux/init.mk
AUTOMAKE_OPTIONS += subdir-objects
TESTS =
include $(top_srcdir)/build-aux/check.mk
include $(top_srcdir)/build-aux/m4sh.mk
include examples/local.mk
include uobjects/local.mk
include checkfiles/local.mk

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

# By default, instrument the code.
INSTRUMENT = true
URBI_SERVER = urbi-console

CHECK_ENVIRONMENT +=				\
  srcdir='$(srcdir)'				\
  INSTRUMENT=$(INSTRUMENT)			\
  PACKAGE_NAME='$(PACKAGE_TARNAME)'		\
  URBI_SERVER=$(URBI_SERVER)

# Use the wrappers to run the non-installed executables.
BUILDCHECK_ENVIRONMENT +=					\
  PATH=$(abs_builddir)/bin:$(abs_top_builddir)/src/examples:$$PATH

INSTALLCHECK_ENVIRONMENT +=			\
  PATH=$(bindir):$$PATH

# By default, tests are buildcheck.
TESTS_ENVIRONMENT += $(BUILDCHECK_ENVIRONMENT)

# Run the tests with the install-environment.
installcheck-local:
	$(MAKE) $(AM_MAKEFLAGS)					\
	  TESTS_ENVIRONMENT=$(INSTALLCHECK_ENVIRONMENT) check

TESTS += $(CHECKFILES) $(UOBJECTS)
TEST_LOGS = $(addsuffix .log,$(basename $(TESTS)))

check-clean-local:
	rm -rf $(TEST_LOGS:.log=.dir)

debug:
	echo $(UOBJECTS)
	echo $(TEST_LOGS)
