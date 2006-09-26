AM_CPPFLAGS =					\
-I$(top_srcdir)/src/liburbi/uobject		\
-I$(top_srcdir)/src/liburbi

AM_LDADD = $(top_builddir)/src/liburbi/liburbi.la $(PTHREAD_LIBS)
# This is a hack for Automake 1.9: 1.10 honors AM_LDADD.
LDADD = $(AM_LDADD)
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)

bin_PROGRAMS =					\
urbiballtrackinghead				\
urbibandwidth					\
urbiimage					\
urbimirror					\
urbiping					\
urbiplay					\
urbirecord					\
urbisend					\
urbisendbin					\
urbisendsound

if SOUNDCARD
bin_PROGRAMS += urbisound
endif

if !WIN32
bin_PROGRAMS += urbitalkie
endif

if WIN32
monitor_sources = monitor-win.h monitor-win.cpp
else
monitor_sources = monitor.h monitor.cpp
endif

urbiballtrackinghead_LDADD = -lX11 -lXext $(AM_LDADD)
urbiballtrackinghead_LDFLAGS = -L/usr/X11R6/lib $(AM_LDFLAGS)
urbiballtrackinghead_SOURCES = urbiballtrackinghead.cpp $(monitor_sources)

urbibandwidth_SOURCES = urbibandwidth.cpp

# Where are the sources of this guy: urbigetbin?
# urbigetbin_SOURCES = urbigetbin.cpp

urbiimage_LDADD = -lX11 -lXext $(AM_LDADD)
urbiimage_LDFLAGS = -L/usr/X11R6/lib $(AM_LDFLAGS)
urbiimage_SOURCES = urbiimage.cpp $(monitor_sources)
urbimirror_SOURCES = urbimirror.cpp
urbiping_SOURCES = urbiping.cpp
urbiplay_SOURCES = urbiplay.cpp
urbirecord_SOURCES = urbirecord.cpp
urbisend_SOURCES = urbisend.cpp
urbisendbin_SOURCES = urbisendbin.cpp
urbisendsound_SOURCES = urbisendsound.cpp
urbisound_SOURCES = urbisound.cpp
urbitalkie_SOURCES = urbitalkie.cpp

