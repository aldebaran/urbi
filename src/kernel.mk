include $(top_srcdir)/build-aux/init.mk

# Find the kernel public headers.
AM_CPPFLAGS += -I$(top_srcdir)/include

# Find the kernel private headers.
AM_CPPFLAGS += -I$(top_srcdir)/src -I$(top_builddir)/src

# Find libport headers.
AM_CPPFLAGS += $(LIBPORT_CPPFLAGS)

# Find uobject headers.
AM_CPPFLAGS += -I$(srcdir)/uobject

AM_CXXFLAGS += $(WARNING_CXXFLAGS) $(PTHREAD_CFLAGS)
