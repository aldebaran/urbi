INCLUDES = -I$(top_srcdir)/src/liburbi
AM_LDFLAGS = -L$(top_srcdir)/src/liburbi 
AM_CXXFLAGS = -pthread

bin_PROGRAMS = urbiping urbibandwidth urbiimage urbisound urbisend urbisendbin urbigetbin urbitalkie urbiplay urbirecord urbimirror urbisendsound urbiballtrackinghead urbispeak

urbiimage_SOURCES = urbiimage.cpp monitor.cpp monitor.h
urbiimage_LDFLAGS = -L/usr/X11R6/lib  
urbiimage_LDADD = -lX11 -lXext -lurbi
urbisound_SOURCES = urbisound.cpp
urbisound_LDADD = -lurbi
urbisend_SOURCES = urbisend.cpp
urbisend_LDADD = -lurbi
urbiplay_SOURCES = urbiplay.cpp
urbiplay_LDADD = -lurbi
urbirecord_SOURCES = urbirecord.cpp
urbirecord_LDADD = -lurbi
urbimirror_SOURCES = urbimirror.cpp
urbimirror_LDADD = -lurbi
urbisendsound_SOURCES = urbisendsound.cpp
urbisendsound_LDADD = -lurbi
urbitalkie_SOURCES = urbitalkie.cpp
urbitalkie_LDADD = -lurbi
urbiping_SOURCES = urbiping.cpp
urbiping_LDADD = -lurbi
urbibandwidth_SOURCES = urbibandwidth.cpp
urbibandwidth_LDADD = -lurbi
urbiballtrackinghead_SOURCES = urbiballtrackinghead.cpp monitor.cpp monitor.h
urbiballtrackinghead_LDADD = -lX11 -lXext -lurbi
urbiballtrackinghead_LDFLAGS = -L/usr/X11R6/lib 
urbisendbin_SOURCES = urbisendbin.cpp
urbisendbin_LDADD = -lurbi
urbigetbin_SOURCES = urbigetbin.cpp
urbigetbin_LDADD = -lurbi

urbispeak_SOURCES = urbispeak.cpp
urbispeak_LDADD = -lurbi
