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
	$(UMAKE_SHARED) --deep-clean ||			\
	  find . -name "_ubuild-*" -a -type d | xargs rm -rf
# This is a bug in deep-clean: we don't clean the .libs files.  But it
# is not so simple, as several builds may share a common .libs, so one
# build cannot remove this directory.
	find . -name ".libs" -a -type d | xargs rm -rf
	rm -f $(uobjects_DATA) $(uobjects_DATA:$(SHLIBEXT)=.la)

# Help to restart broken builds.
$(UMAKE_SHARED):
	make -C tests bin/umake-shared
	make -C $(top_builddir) sdk/umake-shared sdk/umake


## -------------- ##
## Dependencies.  ##
## -------------- ##

# We need to factor this.  .SECONDEXPANSION seems to do what we want,
# but I need to investigate further.
# for i in uobjects/**/*uob
# do
#  printf '%s: $(wildcard $(srcdir)/%s/*)\n' ${i%.uob} $i
# done
uobjects/access-and-change/uaccess$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/access-and-change/uaccess.uob/*)
uobjects/access-and-change/uaccess2$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/access-and-change/uaccess2.uob/*)
uobjects/access-and-change/uaccess3$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/access-and-change/uaccess3.uob/*)
uobjects/access-and-change/uchange$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/access-and-change/uchange.uob/*)
uobjects/all$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/all.uob/*)
uobjects/generic$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/generic.uob/*)
uobjects/lib-urbi$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/liburbi.uob/*)
uobjects/remote$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/remote.uob/*)
uobjects/sensor$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/sensor.uob/*)
uobjects/sensor2$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/sensor2.uob/*)
uobjects/timer$(SHLIBEXT): $(wildcard $(srcdir)/uobjects/timer.uob/*)
