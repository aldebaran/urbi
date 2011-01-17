## Copyright (C) 2008-2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## -------------------- ##
## uobject test suite.  ##
## -------------------- ##

# Compile each UObject using umake.
UOBJECTS =
include $(top_srcdir)/sdk-remote/src/uobjects/test/local.mk
include $(top_srcdir)/sdk-remote/src/uobjects/urbi/local.mk
UOBS = $(patsubst %,uobjects/%.uob,$(UOBJECTS))
UOBJECTS_TESTS = $(UOBS)

# uobject-check
EXTRA_DIST += bin/uobject-check.m4sh
nodist_check_SCRIPTS += bin/uobject-check
m4sh_scripts += bin/uobject-check

$(UOBJECTS_TESTS:.uob=.log): uobjects/%.log: $(sdk_remote_builddir)/src/uobjects/%.la bin/uobject-check
	@$(am__check_pre) bin/uobject-check $< $(am__check_post)

TESTS += $(UOBJECTS_TESTS)

.PHONY: check-uobjects
check-uobjects:
	$(MAKE) $(AM_MAKEFLAGS) check TESTS="$(UOBJECTS_TESTS)"
