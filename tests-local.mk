# This file is loaded by tests/Makefile.am.  It contains things that
# are specific to a given project or even Svn branch.
UCONSOLE_CHECK_FLAGS = -k2

TESTS_DIRS = 0.x 1.x 2.x # uob

URBI_SERVER = $(abs_top_builddir)/src/urbi-console
# Run k2 tests only.
k2-check:
	$(MAKE) check TESTS_DIRS=2.x

# k2 is under heavy development, don't catch SEGV and the like as hard errors
# while running make check.
ENABLE_HARD_ERRORS = false

# k2 tests that fail.
XFAIL_TESTS +=					\
2.x/literal-string.chk

# Tests that are only for 1.x, and we don't want them in k2.
# We should probably use different directories instead.
XFAIL_TESTS +=					\
1.x/string-comparison.chk


# k1 tests that currently don't pass, but we should.
# Not really sorted yet, please help.
XFAIL_TESTS +=					\
1.x/at-is-def.chk				\
1.x/begin-end-report.chk			\
1.x/bin-copy.chk				\
1.x/bin.chk					\
1.x/blocking-comma.chk				\
1.x/channels-tags.chk				\
1.x/connection-delete.chk			\
1.x/delete.chk					\
1.x/doublecolon.chk				\
1.x/echo.chk					\
1.x/eval.chk					\
1.x/events.chk					\
1.x/events-emit-then-return-in-function.chk	\
1.x/every.chk					\
1.x/for-in.chk					\
1.x/freeze-time.chk				\
1.x/function-declaration.chk			\
1.x/function-delete.chk				\
1.x/groups.chk					\
1.x/hash-and-list.chk				\
1.x/hierarchical-tags.chk			\
1.x/illegal-operation-on-void.chk		\
1.x/init-in-pipe-with-time.chk			\
1.x/lazy-test-eval.chk				\
1.x/load.chk					\
1.x/loadwav.chk					\
1.x/long-identifiers.chk			\
1.x/loopn.chk					\
1.x/modifier-accel-adaptive.chk			\
1.x/modifier-accel.chk				\
1.x/modifier-smooth-adaptive.chk		\
1.x/modifier-smooth.chk				\
1.x/modifier-speed-adaptive.chk			\
1.x/modifier-speed.chk				\
1.x/modifier-time-adaptive.chk			\
1.x/modifier-time.chk				\
1.x/modifier-syntax.chk				\
1.x/multiinheritance.chk			\
1.x/nameresolution-array.chk			\
1.x/nameresolution-dollars.chk			\
1.x/nameresolution-objects.chk			\
1.x/nameresolution.chk				\
1.x/new-init.chk				\
1.x/notag.chk					\
1.x/object-events.chk				\
1.x/onleave.chk					\
1.x/onleave2.chk				\
1.x/parse-errors.chk				\
1.x/ping.chk					\
1.x/simple-alias.chk				\
1.x/sinus.chk					\
1.x/speed-zero.chk				\
1.x/stopif.chk					\
1.x/switch.chk					\
1.x/synchro-assignment.chk			\
1.x/tag.chk					\
1.x/undefall.chk				\
1.x/var-lookup.chk				\
1.x/variable-normalize.chk			\
1.x/variable-qualifiers.chk			\
1.x/whenever-bad.chk
