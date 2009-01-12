# Compile each UObject using umake.

include uobjects/uobjects.mk

EXTRA_DIST += $(addprefix uobjects/,$(UOBJECTS:=.uob))

uobjects_DATA = $(addprefix uobjects/,$(UOBJECTS:=$(SHLIBEXT)))

# uobjects_all_SOURCES = uobjects/all.uob
%$(SHLIBEXT): %.uob
	$(top_builddir)/src/tests/bin/umake-shared --output=$@ $<

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
