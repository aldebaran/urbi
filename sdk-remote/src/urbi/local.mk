## Copyright (C) 2009-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

include urbi/uobjects.mk

EXTRA_DIST += $(addprefix urbi/,$(UOBJECTS:=.uob))

# _SCRIPTS so that we preserve the executable bit.
urbi_uobjects_SCRIPTS = $(addprefix urbi/,$(UOBJECTS:=$(DLMODEXT)))
CLEANFILES += $(urbi_uobjects_DATA) $(urbi_uobjects_DATA:$(DLMODEXT)=.la)

UMAKE_SHARED = tests/bin/umake-shared

umake_verbose = $(umake_verbose_$(V))
umake_verbose_ = $(umake_verbose_$(AM_DEFAULT_VERBOSITY))
umake_verbose_0 = @echo UMAKE $@;

UMAKE_VERBOSE = $(UMAKE_VERBOSE_$(V))
UMAKE_VERBOSE_ = $(UMAKE_VERBOSE_$(AM_DEFAULT_VERBOSITY))
UMAKE_VERBOSE_0 = --quiet

%$(DLMODEXT): %.uob $(UMAKE_SHARED) libuobject/libuobject$(LIBSFX).la
	$(umake_verbose)$(UMAKE_SHARED) $(UMAKE_VERBOSE)	\
	  EXTRA_CPPFLAGS="$(EXTRA_$(notdir $*)_cppflags)"	\
	  EXTRA_LDFLAGS="$(EXTRA_$(notdir $*)_ldflags)"		\
	  --clean --output=$@ $< && 				\
## umake has dependencies support, so it might not recompile here, in
## which case, if this was triggered because of $(UMAKE_SHARED) we
## will keep on cycling.
	touch $@

clean-local: clean-uobjects
clean-uobjects:
# If we are unlucky, umake-shared will be cleaned before we call it.
# When clean is concurrent, we might even have "rm" be given
# directories that no longer exist.  So forget about the exit status.
	-$(UMAKE_SHARED) --deep-clean ||			\
	  find urbi -name "_ubuild-*" -a -type d | xargs rm -rf
# This is a bug in deep-clean: we don't clean the .libs files.  But it
# is not so simple, as several builds may share a common .libs, so one
# build cannot remove this directory.
	-find urbi -name ".libs" -a -type d | xargs rm -rf
	-rm -f $(urbi_uobjects_DATA) $(urbi_uobjects_DATA:$(DLMODEXT)=.la)

# Help to restart broken builds.
$(UMAKE_SHARED):
	$(MAKE) $(AM_MAKEFLAGS) -C tests bin/umake-shared
	$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir) sdk/umake-shared sdk/umake


## -------------- ##
## installcheck.  ##
## -------------- ##

# Check that we can clean and make again the UObjects with the
# installed SDK Remote, not the build one.
installcheck installcheck-html: installcheck-umake
installcheck-umake:
	PATH="$(DESTDIR)$(bindir):$$PATH" &&				      \
	  $(MAKE) $(AM_MAKEFLAGS) UMAKE_SHARED=umake-shared clean-uobjects && \
	  $(MAKE) $(AM_MAKEFLAGS) UMAKE_SHARED=umake-shared $(urbi_uobjects_DATA)
