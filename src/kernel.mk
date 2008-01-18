include $(top_srcdir)/libport/build-aux/init.mk
# Get $(libport_la).
include $(top_srcdir)/libport/lib/libport-sources.mk

# Find the kernel public headers.
AM_CPPFLAGS += -I$(top_srcdir)/include

# Find the kernel private headers.  location.hh etc. are generated.
AM_CPPFLAGS += -I$(top_srcdir)/src -I$(top_builddir)/src

# Find the uobject headers
uobject_srcdir = $(top_srcdir)/urbi-sdk-remote/src/liburbi/uobject
AM_CPPFLAGS += -I$(uobject_srcdir)

# Find libport headers.
AM_CPPFLAGS += $(LIBPORT_CPPFLAGS) \
	       -I$(top_srcdir)/libport -I$(top_builddir)/libport

# Find uobject headers.
AM_CPPFLAGS += -I$(srcdir)/uobject

AM_CXXFLAGS += $(WARNING_CXXFLAGS) $(PTHREAD_CFLAGS)

# Add boost libraries.
AM_CPPFLAGS += $(BOOST_CPPFLAGS)
AM_LDFLAGS  += $(BOOST_THREAD_LDFLAGS)
# There is no AM_LIBADD.
LIBS        += $(BOOST_THREAD_LIBS)
