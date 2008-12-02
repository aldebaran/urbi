bin_PROGRAMS +=					\
  examples/urbi-bandwidth			\
  examples/urbi-mirror				\
  examples/urbi-ping				\
  examples/urbi-play				\
  examples/urbi-record				\
  examples/urbi-send				\
  examples/urbi-sendbin				\
  examples/urbi-sendsound

examples_urbi_bandwidth_SOURCES	= examples/urbi-bandwidth.cc
examples_urbi_mirror_SOURCES	= examples/urbi-mirror.cc
examples_urbi_ping_SOURCES	= examples/urbi-ping.cc
examples_urbi_play_SOURCES	= examples/urbi-play.cc
examples_urbi_record_SOURCES	= examples/urbi-record.cc
examples_urbi_send_SOURCES	= examples/urbi-send.cc
examples_urbi_sendbin_SOURCES	= examples/urbi-sendbin.cc
examples_urbi_sendsound_SOURCES	= examples/urbi-sendsound.cc

if  SOUNDCARD
bin_PROGRAMS += examples/urbi-sound
examples_urbi_sound_SOURCES = examples/urbi-sound.cc
endif

if  !WIN32
bin_PROGRAMS += examples/urbi-talkie
examples_urbi_talkie_SOURCES = examples/urbi-talkie.cc
endif



## -------------------------------------- ##
## Examples that require X11 or Windows.  ##
## -------------------------------------- ##

monitor_programs =				\
 examples/urbi-balltrackinghead			\
 examples/urbi-image

if  WIN32
#bin_PROGRAMS += $(monitor_programs)
monitor_sources =				\
  examples/monitor-win.h			\
  examples/monitor-win.cc
X11_LDADD += -lgdi32 -luser32
endif

if  X11
bin_PROGRAMS += $(monitor_programs)
monitor_sources =				\
  examples/monitor.h				\
  examples/monitor.cc
endif

examples_urbi_balltrackinghead_SOURCES =	\
  examples/urbi-balltrackinghead.cc		\
  $(monitor_sources)
examples_urbi_balltrackinghead_CXXFLAGS = $(X_CFLAGS) $(AM_CXXFLAGS)
examples_urbi_balltrackinghead_LDADD = $(X11_LDADD) $(AM_LDADD)

examples_urbi_image_SOURCES =			\
  examples/urbi-image.cc			\
  $(monitor_sources)
examples_urbi_image_CXXFLAGS = $(X_CFLAGS) $(AM_CXXFLAGS)
examples_urbi_image_LDADD = $(X11_LDADD) $(AM_LDADD)
