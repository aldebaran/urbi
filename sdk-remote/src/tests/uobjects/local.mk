## -------------------- ##
## uobject test suite.  ##
## -------------------- ##

# Compile each UObject using umake.
include $(top_srcdir)/src/uobjects/uobjects.mk
UOBJECTS_TESTS = $(addprefix uobjects/, $(UOBJECTS:=.uob))

# uobject-check
EXTRA_DIST += bin/uobject-check.as
nodist_check_SCRIPTS += bin/uobject-check
m4sh_scripts += bin/uobject-check

uobjects/%.log: $(top_builddir)/src/uobjects/%.la bin/uobject-check
	@$(am__check_pre) bin/uobject-check $< $(am__check_post)

TESTS += $(UOBJECTS_TESTS)

.PHONY: check-uobjects
check-uobjects:
	$(MAKE) $(AM_MAKEFLAGS) check TESTS="$(UOBJECTS_TESTS)"
