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
checkfiles/alias.chk				\
checkfiles/arithmetics-failures.chk		\
checkfiles/armor-unarmor.chk			\
checkfiles/basic.chk				\
checkfiles/begin-end-report.chk			\
checkfiles/bin-copy.chk				\
checkfiles/bin.chk				\
checkfiles/bug76.chk				\
checkfiles/class-definition.chk			\
checkfiles/comparison.chk			\
checkfiles/doublecolon.chk			\
checkfiles/events.chk				\
checkfiles/exec.chk				\
checkfiles/freeze-time.chk			\
checkfiles/function-declaration.chk		\
checkfiles/function-delete.chk			\
checkfiles/function-return.chk			\
checkfiles/groups.chk				\
checkfiles/hash-and-list.chk			\
checkfiles/hierarchical-tags.chk		\
checkfiles/illegal-operation-on-void.chk	\
checkfiles/init-in-pipe-with-time.chk		\
checkfiles/lazy-test-eval.chk			\
checkfiles/load.chk				\
checkfiles/long-identifiers.chk			\
checkfiles/multiinheritance.chk			\
checkfiles/nameresolution-array.chk		\
checkfiles/nameresolution-dollars.chk		\
checkfiles/nameresolution-objects.chk		\
checkfiles/nameresolution.chk			\
checkfiles/new-init.chk				\
checkfiles/object-events.chk			\
checkfiles/onleave.chk				\
checkfiles/onleave2.chk				\
checkfiles/parse-errors.chk			\
checkfiles/simple-alias.chk			\
checkfiles/size.chk				\
checkfiles/speed-zero.chk			\
checkfiles/stopif.chk				\
checkfiles/string-escapes.chk			\
checkfiles/switch.chk				\
checkfiles/synchro-assignment.chk		\
checkfiles/tag.chk				\
checkfiles/undefall.chk				\
checkfiles/variable-qualifiers.chk		\
checkfiles/whenever-bad.chk
