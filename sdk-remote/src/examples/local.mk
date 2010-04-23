## Copyright (C) 2008-2010, Gostai S.A.S.
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
  examples/urbi-sendsound

if SOUNDCARD
bin_PROGRAMS += examples/urbi-sound
endif

if !WIN32
bin_PROGRAMS += examples/urbi-talkie
endif



## -------------------------------------- ##
## Examples that require X11 or Windows.  ##
## -------------------------------------- ##

monitor_programs =				\
 examples/urbi-balltrackinghead			\
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

# The -I flags are honored lifo.  So pass X_CFLAGS.

examples_urbi_balltrackinghead_SOURCES =	\
  examples/urbi-balltrackinghead.cc		\
  $(monitor_sources)
examples_urbi_balltrackinghead_CPPFLAGS = $(X11_CPPFLAGS)  $(AM_CPPFLAGS)
examples_urbi_balltrackinghead_LDADD = $(AM_LDADD) $(X11_LDADD)

examples_urbi_image_SOURCES =			\
  examples/urbi-image.cc			\
  $(monitor_sources)
examples_urbi_image_CPPFLAGS = $(X11_CPPFLAGS) $(AM_CPPFLAGS)
examples_urbi_image_LDADD = $(AM_LDADD) $(X11_LDADD)
