## Copyright (C) 2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

if HAVE_ORTP
  UOBJECTS += urbi/rtp
  urbi/rtp$(DLMODEXT): $(wildcard $(srcdir)/uobjects/urbi/rtp.uob/*)
  #use lowercase to avoid confusing automake
  EXTRA_rtp_ldflags=$(ORTP_LIBS)
if  WIN32
    EXTRA_rtp_ldflags += -lwinmm
endif WIN32
  EXTRA_rtp_cppflags=$(ORTP_CFLAGS)
endif

UOBJECTS += urbi/fusion
urbi/fusion$(DLMODEXT): $(wildcard $(srcdir)/uobjects/urbi/fusion.uob/*)
