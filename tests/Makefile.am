include $(top_srcdir)/build-aux/init.mk

# The test suite is composed of *.chk files.  Each one must create a
# *.log file that we want to keep.
TESTS_ENVIRONMENT = srcdir=$(srcdir)
TESTS = $(notdir $(wildcard $(srcdir)/urbivalid/checkfiles/*.chk))
XFAIL_TESTS = \
	events.chk	\
        tag.chk
TEST_LOGS = $(TESTS:.chk=.log)
# .PRECIOUS: $(TEST_LOGS)

# Use a parallel test framework.
include $(top_srcdir)/build-aux/check.mk

# From a test file to a log file.
# FIXME: Find a means to factor...
%.log: $(srcdir)/urbivalid/checkfiles/%.chk
	@if test -f ./$<; then dir=./;			\
	elif test -f $<; then dir=;			\
	else dir="$(srcdir)/"; fi;			\
	$(TESTS_ENVIRONMENT) $(srcdir)/check-file $${dir}$< >$@-t 2>&1;	\
	status=$$?;					\
	case $$status:" $(XFAIL_TESTS) " in		\
	    0:*" $$(basename $<) "*) res='XPASS';;	\
	    0:*)        res='PASS' ;;			\
	    77:*)       res='SKIP' ;;			\
	    *:*" $$(basename $<) "*) res='XFAIL';;	\
	    *:*)        res='FAIL' ;;			\
	   esac;					\
	echo "$$res: $$(basename $<)";			\
	echo "$$res: $$(basename $<) (exit: $$status)" >$@
	@cat $@-t >>$@
	@rm $@-t

EXTRA_DIST += check-file urbivalid
CLEANFILES +=  $(TESTS:.chk=.out) $(TESTS:.chk=.in) $(TESTS:.chk=.expout)

## ---------------------- ##
## Update svn:externals.  ##
## ---------------------- ##
include $(top_srcdir)/build-aux/svn-externals.mk
SVN_EXTERNALS += urbivalid
