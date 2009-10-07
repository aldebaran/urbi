## ------------- ##
## Urbi-launch.  ##
## ------------- ##

bin_PROGRAMS += bin/urbi-launch
# Find our own ltdl.h.  Push "system" headers last so that our headers
# have priority.
bin_urbi_launch_CPPFLAGS = $(LTDL_CPPFLAGS) $(AM_CPPFLAGS)
# We are using Libport.OptionParser which uses Boost.Optional, which is
# incompatible with strict alias analysis.
bin_urbi_launch_CXXFLAGS = -fno-strict-aliasing $(AM_CXXFLAGS)
bin_urbi_launch_SOURCES = bin/urbi-launch.cc
bin_urbi_launch_LDFLAGS =			\
  $(AM_LDFLAGS)
bin_urbi_launch_LDADD =				\
  $(LTDL_LIBS)
