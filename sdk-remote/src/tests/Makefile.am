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

TESTS_ENVIRONMENT +=				\
  srcdir='$(srcdir)'				\
  INSTRUMENT=$(INSTRUMENT)			\
  PACKAGE_NAME='$(PACKAGE_TARNAME)'		\
  URBI_SERVER=$(URBI_SERVER)			\
  PATH=$(abs_builddir)/bin:$(abs_top_builddir)/src/examples:$$PATH


TESTS += $(CHECKFILES) $(UOBJECTS)
TEST_LOGS = $(addsuffix .log,$(basename $(TESTS)))

# Run some tests only.
check-%:
        $(MAKE) $(AM_MAKEFLAGS) check-html TESTS_DIRS=$* VERBOSE=1

check-clean-local:
	rm -rf $(CHECKFILES:.cc=.dir)
