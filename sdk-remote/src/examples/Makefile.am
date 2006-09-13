AM_CPPFLAGS =					\
-I$(top_builddir)/src/liburbi/uobject		\
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
urbisendsound					\
urbisound					\
urbispeak					\
urbitalkie


urbiballtrackinghead_LDADD = -lX11 -lXext $(AM_LDADD)
urbiballtrackinghead_LDFLAGS = -L/usr/X11R6/lib $(AM_LDFLAGS)
urbiballtrackinghead_SOURCES = urbiballtrackinghead.cpp monitor.cpp monitor.h

urbibandwidth_SOURCES = urbibandwidth.cpp

# Where are the sources of this guy: urbigetbin?
# urbigetbin_SOURCES = urbigetbin.cpp

urbiimage_LDADD = -lX11 -lXext $(AM_LDADD)
urbiimage_LDFLAGS = -L/usr/X11R6/lib $(AM_LDFLAGS)
urbiimage_SOURCES = urbiimage.cpp monitor.cpp monitor.h

urbimirror_SOURCES = urbimirror.cpp
urbiping_SOURCES = urbiping.cpp
urbiplay_SOURCES = urbiplay.cpp
urbirecord_SOURCES = urbirecord.cpp
urbisend_SOURCES = urbisend.cpp
urbisendbin_SOURCES = urbisendbin.cpp
urbisendsound_SOURCES = urbisendsound.cpp
urbisound_SOURCES = urbisound.cpp
urbispeak_SOURCES = urbispeak.cpp
urbitalkie_SOURCES = urbitalkie.cpp

