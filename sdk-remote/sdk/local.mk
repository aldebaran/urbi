# libtool is required by umake.
brandlibexec_SCRIPTS += $(top_builddir)/libtool

# This file tells umake how to build things.
nodist_env_DATA = sdk/param.mk

UMAKE_WRAPPERS =                                \
  sdk/umake-deepclean                           \
  sdk/umake-shared

UMAKE_CONFIGURED =                              \
  sdk/umake                                     \
  sdk/umake-link

nodist_bin_SCRIPTS += $(UMAKE_WRAPPERS)
dist_bin_SCRIPTS   += $(UMAKE_CONFIGURED)

$(UMAKE_WRAPPERS): sdk/wrapper.sh
	sdk/wrapper.sh $@
$(UMAKE_CONFIGURED): sdk/umake-common

CLEANFILES += $(nodist_bin_SCRIPTS)

EXTRA_DIST += sdk/umake-common sdk/eval_makefile.pl

## ------------- ##
## param.mk.in.  ##
## ------------- ##

# There are macros that we want to override, we might not want to
# import all the macros from the Makefiles.  It would be simpler though.
#
#$(srcdir)/param.mk.in: $(srcdir)/param.mk.bottom $(srcdir)/configure.ac
#	autoconf -t 'AC_SUBST:$$1 = @$$1@' | sort -u  >$@.tmp
#	cat $(srcdir)/param.mk.bottom                >>$@.tmp
#	mv $@.tmp $@
