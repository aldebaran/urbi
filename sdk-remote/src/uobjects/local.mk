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
uobjects/access-and-change/uaccess: $(wildcard $(srcdir)/uobjects/access-and-change/uaccess.uob/*)
uobjects/access-and-change/uaccess2: $(wildcard $(srcdir)/uobjects/access-and-change/uaccess2.uob/*)
uobjects/access-and-change/uaccess3: $(wildcard $(srcdir)/uobjects/access-and-change/uaccess3.uob/*)
uobjects/access-and-change/uchange: $(wildcard $(srcdir)/uobjects/access-and-change/uchange.uob/*)
uobjects/all: $(wildcard $(srcdir)/uobjects/all.uob/*)
uobjects/generic: $(wildcard $(srcdir)/uobjects/generic.uob/*)
uobjects/lib-urbi: $(wildcard $(srcdir)/uobjects/liburbi.uob/*)
uobjects/remote: $(wildcard $(srcdir)/uobjects/remote.uob/*)
uobjects/sensor: $(wildcard $(srcdir)/uobjects/sensor.uob/*)
uobjects/sensor2: $(wildcard $(srcdir)/uobjects/sensor2.uob/*)
uobjects/timer: $(wildcard $(srcdir)/uobjects/timer.uob/*)

# env_DATA = $($(notdir $(UOBJECTS)):.uob=.la)
# %.la: %.uob
#	umake-shared --output=$@ $<
