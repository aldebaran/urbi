# This file is loaded by tests/Makefile.am.  It contains things that
# are specific to a given project or even Svn branch.
UCONSOLE_CHECK_FLAGS = -k2

K2_TESTS = $(call tests_for,2.x)
TESTS_DIRS += 2.x

# Run k2 tests only.
k2-check:
	$(MAKE) check TESTS="$(K2_TESTS)"

# k2 tests that fail.
XFAIL_TESTS +=					\
2.x/closure.chk

# k1 tests that currently don't pass.
XFAIL_TESTS +=					\
1.x/alias.chk					\
1.x/arithmetics-failures.chk			\
1.x/armor-unarmor.chk				\
1.x/basic.chk					\
1.x/begin-end-report.chk			\
1.x/bin-copy.chk				\
1.x/bin.chk					\
1.x/bug76.chk					\
1.x/class-definition.chk			\
1.x/comparison.chk				\
1.x/doublecolon.chk				\
1.x/events.chk					\
1.x/exec.chk					\
1.x/for-loop.chk				\
1.x/foreach.chk					\
1.x/freeze-time.chk				\
1.x/function-declaration.chk			\
1.x/function-delete.chk				\
1.x/function-return.chk				\
1.x/groups.chk					\
1.x/hash-and-list.chk				\
1.x/hierarchical-tags.chk			\
1.x/illegal-operation-on-void.chk		\
1.x/init-in-pipe-with-time.chk			\
1.x/lazy-test-eval.chk				\
1.x/load.chk					\
1.x/long-identifiers.chk			\
1.x/loopn.chk					\
1.x/multiinheritance.chk			\
1.x/nameresolution-array.chk			\
1.x/nameresolution-dollars.chk			\
1.x/nameresolution-objects.chk			\
1.x/nameresolution.chk				\
1.x/new-init.chk				\
1.x/object-events.chk				\
1.x/onleave.chk					\
1.x/onleave2.chk				\
1.x/parse-errors.chk				\
1.x/simple-alias.chk				\
1.x/size.chk					\
1.x/speed-zero.chk				\
1.x/stopif.chk					\
1.x/string-escapes.chk				\
1.x/switch.chk					\
1.x/synchro-assignment.chk			\
1.x/tag.chk					\
1.x/undefall.chk				\
1.x/variable-qualifiers.chk			\
1.x/whenever-bad.chk
