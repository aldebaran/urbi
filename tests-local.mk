# This file is loaded by tests/Makefile.am.  It contains things that
# are specific to a given project or even Svn branch.
UCONSOLE_CHECK_FLAGS = -k2

TESTS_DIRS = 0.x 1.x 2.x uob

URBI_SERVER = $(abs_top_builddir)/src/urbi-console
# Run k2 tests only.
k2-check:
	$(MAKE) check TESTS_DIRS=2.x

# k2 is under heavy development, don't catch SEGV and the like as hard errors
# while running make check.
ENABLE_HARD_ERRORS = false

# k1 tests that fail currently but need to be fixed soon.
TFAIL_TESTS +=                                  \
1.x/eval.chk

# k2 tests that fail.
XFAIL_TESTS +=					\
2.x/literal-string.chk

# pending features
TFAIL_TESTS +=					\
2.x/onevent.chk					\
2.x/list-primitives.chk				\
2.x/unique-variable.chk                         \
2.x/modifier/speed-adaptive-freeze.log

# k1 tests that currently don't pass, but we should.
# In fact, the above list has been removed to gain some time
# when running tests. The following tests should be examined
# and reintroduced in 1.x as needed.
TO_CHECK_TESTS =				\
1.-/begin-end-report.chk			\
1.-/channels-tags.chk				\
1.-/connection-delete.chk			\
1.-/events.chk					\
1.-/events-emit-then-return-in-function.chk	\
1.-/every.chk					\
1.-/every-emit.chk				\
1.-/freeze-time.chk				\
1.-/function-delete.chk                         \
1.-/hash-and-list.chk				\
1.-/hierarchical-tags.chk			\
1.-/init-in-pipe-with-time.chk			\
1.-/lazy-test-eval.chk				\
1.-/loadwav.chk					\
1.-/modifier/accel-adaptive.chk			\
1.-/modifier/accel.chk				\
1.-/modifier/smooth-adaptive.chk		\
1.-/modifier/smooth.chk				\
1.-/modifier/speed-adaptive.chk			\
1.-/modifier/speed.chk				\
1.-/modifier/time-adaptive.chk			\
1.-/modifier/time.chk				\
1.-/modifier/syntax.chk				\
1.-/nameresolution-array.chk			\
1.-/nameresolution-dollar-computed.chk		\
1.-/nameresolution-dollars.chk			\
1.-/nameresolution-objects.chk			\
1.-/object-events.chk				\
1.-/sinus.chk					\
1.-/speed-zero.chk				\
1.-/synchro-assignment.chk			\
1.-/tag.chk					\
1.-/tag-reduce.chk                              \
1.-/variable-normalize.chk			\
1.-/variable-qualifiers.chk

# Uobject tests that we fail because features are not implemented
XFAIL_TESTS +=                                  \
uob/group.chk                                   \
uob/all-write-plug-prop.chk                     \
uob/remote/all-write-prop.chk
