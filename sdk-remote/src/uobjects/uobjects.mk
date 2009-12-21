# I don't know yet how to avoid this painful list.
UOBJECTS =					\
  access-and-change/uaccess			\
  access-and-change/uaccess2			\
  access-and-change/uaccess3			\
  access-and-change/uchange			\
  all						\
  factory/factory				\
  generic					\
  lib-urbi					\
  remote					\
  sensor					\
  sensor2					\
  timer						\
  ultest

## -------------- ##
## Dependencies.  ##
## -------------- ##

# We need to factor this.  .SECONDEXPANSION seems to do what we want,
# but I need to investigate further.
# for i in uobjects/**/*uob
# do
#  printf '%s: $(wildcard $(srcdir)/%s/*)\n' ${i%.uob} $i
# done
uobjects/access-and-change/uaccess$(DLMODEXT): $(wildcard $(srcdir)/uobjects/access-and-change/uaccess.uob/*)
uobjects/access-and-change/uaccess2$(DLMODEXT): $(wildcard $(srcdir)/uobjects/access-and-change/uaccess2.uob/*)
uobjects/access-and-change/uaccess3$(DLMODEXT): $(wildcard $(srcdir)/uobjects/access-and-change/uaccess3.uob/*)
uobjects/access-and-change/uchange$(DLMODEXT): $(wildcard $(srcdir)/uobjects/access-and-change/uchange.uob/*)
uobjects/all$(DLMODEXT): $(wildcard $(srcdir)/uobjects/all.uob/*)
uobjects/factory/factory$(DLMODEXT): $(wildcard $(srcdir)/uobjects/factory/factory.uob/*)
uobjects/generic$(DLMODEXT): $(wildcard $(srcdir)/uobjects/generic.uob/*)
uobjects/lib-urbi$(DLMODEXT): $(wildcard $(srcdir)/uobjects/liburbi.uob/*)
uobjects/remote$(DLMODEXT): $(wildcard $(srcdir)/uobjects/remote.uob/*)
uobjects/sensor$(DLMODEXT): $(wildcard $(srcdir)/uobjects/sensor.uob/*)
uobjects/sensor2$(DLMODEXT): $(wildcard $(srcdir)/uobjects/sensor2.uob/*)
uobjects/timer$(DLMODEXT): $(wildcard $(srcdir)/uobjects/timer.uob/*)
uobjects/ultest$(DLMODEXT): $(wildcard $(srcdir)/uobjects/ultest.uob/*)
