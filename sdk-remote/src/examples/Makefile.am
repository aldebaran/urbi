AM_CPPFLAGS =					\
-I$(top_srcdir)					\
-I$(top_srcdir)/src/liburbi/uobject		\
-I$(top_srcdir)/src/liburbi

AM_LDADD = $(top_builddir)/src/liburbi/liburbi.la $(PTHREAD_LIBS)
# This is a hack for Automake 1.9: 1.10 honors AM_LDADD.
LDADD = $(AM_LDADD)
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)

bin_PROGRAMS =					\
urbibandwidth					\
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

urbibandwidth_SOURCES = urbibandwidth.cc
urbimirror_SOURCES = urbimirror.cc
urbiping_SOURCES = urbiping.cc
urbiplay_SOURCES = urbiplay.cc
urbirecord_SOURCES = urbirecord.cc
urbisend_SOURCES = urbisend.cc
urbisendbin_SOURCES = urbisendbin.cc
urbisendsound_SOURCES = urbisendsound.cc
urbisound_SOURCES = urbisound.cc
urbitalkie_SOURCES = urbitalkie.cc


## -------------------------------------- ##
## Examples that require X11 or Windows.  ##
## -------------------------------------- ##

monitor_programs = 				\
urbiballtrackinghead				\
urbiimage

if WIN32
bin_PROGRAMS += $(monitor_programs)
monitor_sources = monitor-win.h monitor-win.cc
endif

if X11
bin_PROGRAMS += $(monitor_programs)
monitor_sources = monitor.h monitor.cc
endif

# FIXME: -lXext should be checked via configure.ac.
x11_libs = -L/usr/X11R6/lib $(X_PRE_LIBS) $(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS) $(AM_LDADD)

urbiballtrackinghead_SOURCES = urbiballtrackinghead.cc $(monitor_sources)
urbiballtrackinghead_CXXFLAGS = $(X_CFLAGS) $(AM_CXXFLAGS)
urbiballtrackinghead_LDADD = $(x11_libs)

urbiimage_SOURCES = urbiimage.cc $(monitor_sources)
urbiimage_CXXFLAGS = $(X_CFLAGS) $(AM_CXXFLAGS)
urbiimage_LDADD = $(x11_libs)
