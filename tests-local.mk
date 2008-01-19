# This file is loaded by tests/Makefile.am.  It contains things that
# are specific to a given project or even Git branch. 

TESTS_DIRS = 0.x 1.5 1.6 1.x uob
URBI_SERVER = $(abs_top_builddir)/src/urbi-console
XFAIL_TESTS += 1.x/every-emit.chk
