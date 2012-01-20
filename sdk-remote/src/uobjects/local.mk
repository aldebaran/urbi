## Copyright (C) 2009-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

UOBJECTS =
include uobjects/urbi/local.mk
include uobjects/test/local.mk
UOBS = $(patsubst %,uobjects/%.uob,$(UOBJECTS))

EXTRA_DIST += $(UOBS)

# _SCRIPTS so that we preserve the executable bit.
urbi_uobjects_SCRIPTS = $(filter uobjects/urbi/%, $(UOBS:.uob=$(DLMODEXT)))
test_uobjects_SCRIPTS = $(filter uobjects/test/%, $(UOBS:.uob=$(DLMODEXT)))

UOBS_LIBS = $(UOBS:.uob=$(DLMODEXT)) $(UOBS:.uob=.la)

CLEANFILES += $(UOBS_LIBS)

UMAKE_SHARED = tests/bin/umake-shared

umake_verbose = $(umake_verbose_$(V))
umake_verbose_ = $(umake_verbose_$(AM_DEFAULT_VERBOSITY))
umake_verbose_0 = @echo "  UMAKE " $@;

UMAKE_VERBOSE = $(UMAKE_VERBOSE_$(V))
UMAKE_VERBOSE_ = $(UMAKE_VERBOSE_$(AM_DEFAULT_VERBOSITY))
UMAKE_VERBOSE_0 = --quiet

UMAKE_SHARED_deps =				\
  $(UMAKE_SHARED)				\
  $(sdk_remote_builddir)/sdk/umake-shared	\
  $(sdk_remote_builddir)/sdk/umake		\
  libuobject/libuobject$(LIBSFX).la
%$(DLMODEXT): %.uob $(UMAKE_SHARED_deps)
	+$(umake_verbose)$(UMAKE_SHARED) $(UMAKE_VERBOSE)	\
	  EXTRA_CXXFLAGS="$(WARNING_CXXFLAGS)"			\
	  $(EXTRA_$(notdir $*)_cppflags)			\
	  $(EXTRA_$(notdir $*)_ldflags)				\
	  --clean --output=$@ $< && 				\
## umake has dependencies support, so it might not recompile here, in
## which case, if this was triggered because of $(UMAKE_SHARED) we
## will keep on cycling.
	touch $@

CLEAN_LOCAL += clean-uobjects
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
	-rm -f $(UOBS) $(UOBS:$(DLMODEXT)=.la)

# Help to restart broken builds.
$(sdk_remote_builddir)/sdk/umake-shared $(sdk_remote_builddir)/sdk/umake \
  : $(sdk_remote_srcdir)/sdk/umake.in
	$(MAKE) $(AM_MAKEFLAGS) -C $(sdk_remote_builddir)	\
	  sdk/umake-shared sdk/umake

$(UMAKE_SHARED): tests/bin/wrapper.in $(sdk_remote_srcdir)/sdk/umake.in
	$(MAKE) $(AM_MAKEFLAGS) -C tests bin/umake-shared


## -------------- ##
## installcheck.  ##
## -------------- ##

# Check that we can clean and make again the UObjects with the
# installed SDK Remote, not the build one.
installcheck installcheck-html: installcheck-umake
installcheck-umake:
	PATH="$(DESTDIR)$(bindir):$$PATH" &&				      \
	  $(MAKE) $(AM_MAKEFLAGS) UMAKE_SHARED=umake-shared clean-uobjects && \
	  $(MAKE) $(AM_MAKEFLAGS) UMAKE_SHARED=umake-shared $(UOBS)
