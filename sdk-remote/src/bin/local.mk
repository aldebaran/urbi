## ------------- ##
## Urbi-launch.  ##
## ------------- ##

bin_PROGRAMS += bin/urbi-launch
# Find our own ltdl.h.  Push "system" headers last so that our headers
# have priority.
bin_urbi_launch_CPPFLAGS = -DURBI_ROOT_NOT_DLL $(LTDL_CPPFLAGS) $(AM_CPPFLAGS)
bin_urbi_launch_SOURCES  = bin/urbi-launch.cc liburbi/urbi-root.cc
bin_urbi_launch_LDFLAGS  = $(AM_LDFLAGS)
bin_urbi_launch_LDADD    = $(LTDL_LIBS)
