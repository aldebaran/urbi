## -------------------- ##
## uobject test suite.  ##
## -------------------- ##

# Compile each UObject using umake.

# I couldn't get git-ls-files to list directories.  So ask for the CC
# files in them, and keep only the *.uob part.
UOBJECTS = $(patsubst %/,%,$(sort $(dir $(call ls_files,uobjects/*.uob/*.cc))))

EXTRA_DIST += $(UOBJECTS)

# uobject-check
EXTRA_DIST += bin/uobject-check.as
nodist_check_SCRIPTS += bin/uobject-check
m4sh_scripts += bin/uobject-check


%.log: %.uob bin/uobject-check
	@$(am__check_pre) bin/uobject-check $(srcdir)/$* $(am__check_post)

debug:
	echo $(UOBJECTS)
	echo $(TEST_LOGS)
