## Copyright (C) 2009-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

# I don't know yet how to avoid this painful list.
UOBJECTS =					\
  uaccess					\
  uaccess2					\
  uaccess3					\
  uchange					\
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

if HAVE_ORTP
  UOBJECTS += rtp
  urbi/rtp$(DLMODEXT): $(wildcard $(srcdir)/urbi/rtp.uob/*)
  #use lowercase to avoid confusing automake
  EXTRA_rtp_ldflags=$(ORTP_LIBS)
if  WIN32
    EXTRA_rtp_ldflags += -lwinmm
endif WIN32
  EXTRA_rtp_cppflags=$(ORTP_CFLAGS)
endif

## -------------- ##
## Dependencies.  ##
## -------------- ##

# We need to factor this.  .SECONDEXPANSION seems to do what we want,
# but I need to investigate further.
# for i in urbi/**/*uob
# do
#  printf '%s: $(wildcard $(srcdir)/%s/*)\n' ${i%.uob} $i
# done
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
urbi/uaccess$(DLMODEXT): $(wildcard $(srcdir)/urbi/uaccess.uob/*)
urbi/uaccess2$(DLMODEXT): $(wildcard $(srcdir)/urbi/uaccess2.uob/*)
urbi/uaccess3$(DLMODEXT): $(wildcard $(srcdir)/urbi/uaccess3.uob/*)
urbi/uchange$(DLMODEXT): $(wildcard $(srcdir)/urbi/uchange.uob/*)
urbi/ultest$(DLMODEXT): $(wildcard $(srcdir)/urbi/ultest.uob/*)
