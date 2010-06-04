## Copyright (C) 2009-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

# I don't know yet how to avoid this painful list.
UOBJECTS =					\
  access-and-change/uaccess			\
  access-and-change/uaccess2			\
  access-and-change/uaccess3			\
  access-and-change/uchange			\
  all						\
  factory					\
  generic					\
  lib-urbi					\
  remote					\
  sensor					\
  sensor2					\
  subsumption					\
  threaded					\
  timer						\
  ultest

## -------------- ##
## Dependencies.  ##
## -------------- ##

# We need to factor this.  .SECONDEXPANSION seems to do what we want,
# but I need to investigate further.
# for i in urbi/**/*uob
# do
#  printf '%s: $(wildcard $(srcdir)/%s/*)\n' ${i%.uob} $i
# done
urbi/access-and-change/uaccess$(DLMODEXT): $(wildcard $(srcdir)/urbi/access-and-change/uaccess.uob/*)
urbi/access-and-change/uaccess2$(DLMODEXT): $(wildcard $(srcdir)/urbi/access-and-change/uaccess2.uob/*)
urbi/access-and-change/uaccess3$(DLMODEXT): $(wildcard $(srcdir)/urbi/access-and-change/uaccess3.uob/*)
urbi/access-and-change/uchange$(DLMODEXT): $(wildcard $(srcdir)/urbi/access-and-change/uchange.uob/*)
urbi/all$(DLMODEXT): $(wildcard $(srcdir)/urbi/all.uob/*)
urbi/factory$(DLMODEXT): $(wildcard $(srcdir)/urbi/factory.uob/*)
urbi/generic$(DLMODEXT): $(wildcard $(srcdir)/urbi/generic.uob/*)
urbi/lib-urbi$(DLMODEXT): $(wildcard $(srcdir)/urbi/liburbi.uob/*)
urbi/remote$(DLMODEXT): $(wildcard $(srcdir)/urbi/remote.uob/*)
urbi/sensor$(DLMODEXT): $(wildcard $(srcdir)/urbi/sensor.uob/*)
urbi/sensor2$(DLMODEXT): $(wildcard $(srcdir)/urbi/sensor2.uob/*)
urbi/subsumption$(DLMODEXT): $(wildcard $(srcdir)/urbi/subsumption.uob/*)
urbi/threaded$(DLMODEXT): $(wildcard $(srcdir)/urbi/threaded.uob/*)
urbi/timer$(DLMODEXT): $(wildcard $(srcdir)/urbi/timer.uob/*)
urbi/ultest$(DLMODEXT): $(wildcard $(srcdir)/urbi/ultest.uob/*)
