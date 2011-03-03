## Copyright (C) 2008-2011, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

bin_PROGRAMS +=					\
  examples/urbi-bandwidth			\
  examples/urbi-mirror				\
  examples/urbi-ping				\
  examples/urbi-play				\
  examples/urbi-record				\
  examples/urbi-send				\
  examples/urbi-sendbin				\
  examples/urbi-sendsound			\
  examples/urbi-sound

if !WIN32
bin_PROGRAMS += examples/urbi-talkie
endif

# We need kernel/config.h.
examples_urbi_sound_CPPFLAGS =			\
  $(AM_CPPFLAGS)				\
  -I$(kernel_builddir)/src

## -------------------------------------- ##
## Examples that require X11 or Windows.  ##
## -------------------------------------- ##

monitor_programs =				\
 examples/urbi-balltracking			\
 examples/urbi-image

if WIN32
bin_PROGRAMS += $(monitor_programs)
monitor_sources =				\
  examples/monitor-win.hh			\
  examples/monitor-win.cc
X11_LDADD += -lgdi32 -luser32
endif

if X11
bin_PROGRAMS += $(monitor_programs)
monitor_sources =				\
  examples/monitor.hh				\
  examples/monitor.cc
endif

# Pass the X11 flags after cppflags so that we have a chance to use
# our boost rather than the system one if there is one that lives in
# the same place as X11.
examples_urbi_balltracking_SOURCES =	\
  examples/urbi-balltracking.cc		\
  $(monitor_sources)
examples_urbi_balltracking_CPPFLAGS = $(AM_CPPFLAGS) $(X11_CPPFLAGS)
examples_urbi_balltracking_LDADD = $(AM_LDADD) $(X11_LDADD)

examples_urbi_image_SOURCES =			\
  examples/urbi-image.cc			\
  $(monitor_sources)
examples_urbi_image_CPPFLAGS = $(AM_CPPFLAGS) $(X11_CPPFLAGS)
examples_urbi_image_LDADD = $(AM_LDADD) $(X11_LDADD)
