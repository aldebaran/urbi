## Copyright (C) 2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

# I don't know yet how to avoid this painful list.
UOBJECTS +=					\
  test/all					\
  test/generic					\
  test/issue-3699				\
  test/lib-urbi					\
  test/machine					\
  test/remote					\
  test/sensor					\
  test/sensor2					\
  test/subsumption				\
  test/threaded					\
  test/timer					\
  test/uaccess					\
  test/uaccess2					\
  test/uaccess3					\
  test/uchange					\
  test/ultest

## -------------- ##
## Dependencies.  ##
## -------------- ##

# We need to factor this.  .SECONDEXPANSION seems to do what we want,
# but I need to investigate further.
# for i in urbi/**/*uob
# do
#  printf '%s: $(wildcard $(srcdir)/%s/*)\n' ${i%.uob} $i
# done
uobjects/test/all$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/all.uob/*)
uobjects/test/generic$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/generic.uob/*)
uobjects/test/issue-3699(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/issue-3699.uob/*)
uobjects/test/lib-urbi$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/liburbi.uob/*)
uobjects/test/machine$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/machine.uob/*)
uobjects/test/remote$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/remote.uob/*)
uobjects/test/sensor$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/sensor.uob/*)
uobjects/test/sensor2$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/sensor2.uob/*)
uobjects/test/subsumption$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/subsumption.uob/*)
uobjects/test/threaded$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/threaded.uob/*)
uobjects/test/timer$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/timer.uob/*)
uobjects/test/uaccess$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/uaccess.uob/*)
uobjects/test/uaccess2$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/uaccess2.uob/*)
uobjects/test/uaccess3$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/uaccess3.uob/*)
uobjects/test/uchange$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/uchange.uob/*)
uobjects/test/ultest$(DLMODEXT): $(wildcard $(srcdir)/uobjects/test/ultest.uob/*)
