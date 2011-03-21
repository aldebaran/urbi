## Copyright (C) 2008, 2010, 2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

# libtool is required by umake.
brandlibexec_SCRIPTS += $(top_builddir)/libtool

# This file tells umake how to build things.
nodist_remote_DATA = sdk/param.mk

javaremotedir = $(remotedir)/java
nodist_javaremote_DATA = sdk/java/param.mk

UMAKE_WRAPPERS =                                \
  sdk/umake-deepclean                           \
  sdk/umake-shared

UMAKE_CONFIGURED =                              \
  sdk/umake                                     \
  sdk/umake-link				\
  sdk/umake-java				\
  sdk/urbi-launch-java

nodist_bin_SCRIPTS += $(UMAKE_WRAPPERS)
dist_bin_SCRIPTS   += $(UMAKE_CONFIGURED)

$(UMAKE_WRAPPERS): sdk/wrapper.sh
	$(AM_V_GEN) sdk/wrapper.sh $@
$(UMAKE_CONFIGURED): sdk/umake-common

$(UMAKE_CONFIGURED): %: %.in
	cd $(top_builddir) && $(SHELL) ./config.status $(subdir)/$@

CLEANFILES += $(nodist_bin_SCRIPTS)

EXTRA_DIST += sdk/umake-common

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
