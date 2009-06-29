# This file is loaded by tests/Makefile.am.  It contains things that
# are specific to a given project or even Svn branch.
UCONSOLE_CHECKFLAGS += --kernel 2

# The test directories we are interested in.
TESTS_DIRS = 0.x 1.x 2.x demo uob

# Run server in fast mode
FAST_MODE = true
# Whether we use Valgrind etc. Activated in 'check-buildfarm'.
INSTRUMENT = false

# The wrappers around our embedded SDK-Remote tools.
sdk_remote_builddir = $(abs_top_builddir)/sdk-remote

# Environment used both in check and installcheck.
CHECK_ENVIRONMENT +=				\
  FAST_MODE=$(FAST_MODE)			\
  INSTRUMENT=$(INSTRUMENT)

# Currently with Wine, we found no (simple) means to execute a Unix
# process from a Windows process.  As a result, the "urbi" native
# process fails to launch properly the urbi-launch wrapper.
# Therefore, we now stop trying to use "urbi" in the test-suite (at
# least in build, during installcheck we should use the binary) and
# directly invoke urbi-launch.
URBI_SERVER = urbi-launch$(EXEEXT) --start --

# Run k2 tests only.
k2-check:
	$(MAKE) check TESTS_DIRS=2.x

# k2 is under heavy development, don't catch SEGV and the like as hard errors
# while running make check.
ENABLE_HARD_ERRORS = false

# test we don't want to care about temporarily
TFAIL_TESTS +=			\
  2.x/every-exception.chk

# k1 tests that currently don't pass, but we should.
# In fact, the above list has been removed to gain some time
# when running tests. The following tests should be examined
# and reintroduced in 1.x as needed.
# TO_CHECK_TESTS =
# # +begin, +end
# 1.-/begin-end-report.chk
# # chan << echo(foo) goes in chan
# 1.-/channels-tags.chk
# # +connection
# 1.-/connection-delete.chk
# # emit foo
# 1.-/events.chk
# 1.-/events-emit-then-return-in-function.chk
# 1.-/every-emit.chk
# # []-access to list and hash
# 1.-/hash-and-list.chk
# # what should we do with very large literals?
# 1.-/invalid-number.chk
# # events
# 1.-/lazy-test-eval.chk
# 1.-/loadwav.chk
# 1.-/object-events.chk

# Uobject tests that we fail because features are not implemented
XFAIL_TESTS +=					\
  2.x/derive.chk				\
  2.x/uob/group.chk
