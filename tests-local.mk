# This file is loaded by tests/Makefile.am.  It contains things that
# are specific to a given project or even Svn branch.

K2_TESTS = $(notdir $(wildcard $(srcdir)/k2/*.chk))
TESTS += $(K2_TESTS)

# Run k2 tests only.
k2-check:
	$(MAKE) check TESTS="$(K2_TESTS)"

# k2 tests that fail.
XFAIL_TESTS +=

# k1 tests that currently don't pass.
XFAIL_TESTS +=					\
alias.chk					\
arithmetics-failures.chk			\
armor-unarmor.chk				\
bad-if.chk					\
bad-whenever.chk				\
basic.chk					\
begin-end-report.chk				\
bin.chk						\
bin-copy.chk					\
bug76.chk					\
comparison.chk					\
doublecolon.chk					\
empty-then.chk					\
events.chk					\
exec.chk					\
for-loop.chk					\
freeze-time.chk					\
function-delete.chk				\
function-return.chk				\
groups.chk					\
hash-and-list.chk				\
hierarchical-tags.chk				\
illegal-operation-on-void.chk			\
init-in-pipe-with-time.chk			\
lazy-test-eval.chk				\
load.chk					\
long-identifiers.chk				\
multiinheritance.chk				\
nameresolution.chk				\
nameresolution-array.chk			\
nameresolution-dollars.chk			\
nameresolution-objects.chk			\
new-init.chk					\
object-events.chk				\
onleave.chk					\
onleave2.chk					\
parse-errors.chk				\
simple-alias.chk				\
size.chk					\
speed-zero.chk					\
string-escapes.chk				\
switch.chk					\
synchro-assignment.chk				\
tag.chk						\
undefall.chk					\
variable-qualifiers.chk
