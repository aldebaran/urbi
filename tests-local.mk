# This file is loaded by tests/Makefile.am.  It contains things that
# are specific to a given project or even Svn branch.

K2_TESTS = $(notdir $(wildcard $(srcdir)/k2/*.chk))
TESTS += $(K2_TESTS)

# Run k2 tests only.
k2-check:
	$(MAKE) check TESTS="$(K2_TESTS)"

XFAIL_TESTS += \
alias.chk			\
arithmetics-failures.chk			\
armor-simple.chk			\
armor_unarmor.chk			\
bad-if.chk			\
bad-whenever.chk			\
basic.chk			\
begin_end_report.chk			\
bin.chk			\
bin_copy.chk			\
bug76.chk			\
class-definition.chk			\
comparison.chk			\
doublecolon.chk			\
empty_then.chk			\
events.chk			\
exec.chk			\
for-loop.chk			\
freeze_time.chk			\
function_delete.chk			\
function_return.chk			\
groups.chk			\
hash_and_list.chk			\
hierarchical_tags.chk			\
illegal_operation_on_void.chk			\
init_in_pipe_with_time.chk			\
lazy-test-eval.chk			\
load.chk			\
long-identifiers.chk			\
multiinheritance.chk			\
nameresolution.chk			\
nameresolution_array.chk			\
nameresolution_dollars.chk			\
nameresolution_objects.chk			\
new_init.chk			\
object_events.chk			\
onleave.chk			\
onleave2.chk			\
parse-errors.chk			\
simple-alias.chk			\
size.chk			\
speed_zero.chk			\
switch.chk			\
synchro_assignment.chk			\
tag.chk			\
undefall.chk			\
variable-qualifiers.chk
