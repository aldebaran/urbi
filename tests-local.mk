# This file is loaded by tests/Makefile.am.  It contains things that
# are specific to a given project or even Svn branch.
UCONSOLE_CHECKFLAGS +=

# The test directories we are interested in.
TESTS_DIRS = 0.x 1.x 2.x demo uob @fsm @ros

# Run server in fast mode
FAST_MODE = true
# Whether we use Valgrind etc.
INSTRUMENTATION = false

# The wrappers around our embedded SDK-Remote tools.
sdk_remote_builddir = $(abs_top_builddir)/sdk-remote
sdk_remote_srcdir = $(abs_top_srcdir)/sdk-remote

# Environment used both in check and installcheck.
CHECK_ENVIRONMENT +=				\
  FAST_MODE=$(FAST_MODE)			\
  INSTRUMENTATION=$(INSTRUMENTATION)

# Currently with Wine, we found no (simple) means to execute a Unix
# process from a Windows process.  As a result, the "urbi" native
# process fails to launch properly the urbi-launch wrapper.
# Therefore, we now stop trying to use "urbi" in the test-suite (at
# least in build, during installcheck we should use the binary) and
# directly invoke urbi-launch.
URBI_SERVER = urbi-launch$(EXEEXT) --start --

# Run k2 tests only.
k2-check:
	@$(MAKE) $(AM_MAKEFLAGS) check TESTS_DIRS=2.x

# Uobject tests that we fail because features are not implemented
XFAIL_TESTS +=					\
  2.x/at/tags.chk				\
  2.x/derive.chk				\
  2.x/uob/group.chk

# Removing laziness
XFAIL_TESTS +=					\
  2.x/lazy-init.chk

XFAIL_TESTS +=						\
  uob/java/all/change-bypass-external-access.chk	\
  uob/java/all/change-bypass.chk			\
  uob/java/all/transmit-object.chk

# Memory leaks which have to be fixed.
TFAIL_TESTS +=					\
  2.x/tfail-mem-check.chk
