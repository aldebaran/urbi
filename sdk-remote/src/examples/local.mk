if EXAMPLES
bin_PROGRAMS +=					\
  examples/urbibandwidth			\
  examples/urbimirror				\
  examples/urbiping				\
  examples/urbiplay				\
  examples/urbirecord				\
  examples/urbisend				\
  examples/urbisendbin				\
  examples/urbisendsound

examples_urbibandwidth_SOURCES	= examples/urbibandwidth.cc
examples_urbimirror_SOURCES	= examples/urbimirror.cc
examples_urbiping_SOURCES	= examples/urbiping.cc
examples_urbiplay_SOURCES	= examples/urbiplay.cc
examples_urbirecord_SOURCES	= examples/urbirecord.cc
examples_urbisend_SOURCES	= examples/urbisend.cc
examples_urbisendbin_SOURCES	= examples/urbisendbin.cc
examples_urbisendsound_SOURCES	= examples/urbisendsound.cc

if  SOUNDCARD
bin_PROGRAMS += examples/urbisound
examples_urbisound_SOURCES = examples/urbisound.cc
endif

if  !WIN32
bin_PROGRAMS += examples/urbitalkie
examples_urbitalkie_SOURCES = examples/urbitalkie.cc
endif
endif EXAMPLES



## -------------------------------------- ##
## Examples that require X11 or Windows.  ##
## -------------------------------------- ##

monitor_programs =				\
 examples/urbiballtrackinghead			\
 examples/urbiimage

if EXAMPLES
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
endif EXAMPLES

examples_urbiballtrackinghead_SOURCES =		\
   examples/urbiballtrackinghead.cc		\
   $(monitor_sources)
examples_urbiballtrackinghead_CXXFLAGS = $(X_CFLAGS) $(AM_CXXFLAGS)
examples_urbiballtrackinghead_LDADD = $(X11_LDADD) $(AM_LDADD)

examples_urbiimage_SOURCES =			\
  examples/urbiimage.cc				\
  $(monitor_sources)
examples_urbiimage_CXXFLAGS = $(X_CFLAGS) $(AM_CXXFLAGS)
examples_urbiimage_LDADD = $(X11_LDADD) $(AM_LDADD)
