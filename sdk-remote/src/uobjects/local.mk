include uobjects/uobjects.mk

EXTRA_DIST += $(addprefix uobjects/,$(UOBJECTS:=.uob))

uobjects_DATA = $(addprefix uobjects/,$(UOBJECTS:=$(SHLIBEXT)))
CLEANFILES += $(uobjects_DATA) $(uobjects_DATA:$(SHLIBEXT)=.la)

UMAKE_SHARED = tests/bin/umake-shared

%$(SHLIBEXT): %.uob $(UMAKE_SHARED) libuobject/libuobject.la
	$(UMAKE_SHARED) --clean --output=$@ $<
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
	  find . -name "_ubuild-*" -a -type d | xargs rm -rf
# This is a bug in deep-clean: we don't clean the .libs files.  But it
# is not so simple, as several builds may share a common .libs, so one
# build cannot remove this directory.
	-find . -name ".libs" -a -type d | xargs rm -rf
	-rm -f $(uobjects_DATA) $(uobjects_DATA:$(SHLIBEXT)=.la)

# Help to restart broken builds.
$(UMAKE_SHARED):
	$(MAKE) $(AM_MAKEFLAGS) -C tests bin/umake-shared
	$(MAKE) $(AM_MAKEFLAGS) -C $(top_builddir) sdk/umake-shared sdk/umake
