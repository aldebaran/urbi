AM_CPPFLAGS =					\
-I$(top_srcdir)/src/liburbi/uobject		\
-I$(top_srcdir)/src/liburbi
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(WARNING_CXXFLAGS)

bin_PROGRAMS = urbicycle urbireverse urbiwalk urbiscale

urbicycle_SOURCES = urbicycle.cpp
urbireverse_SOURCES = urbireverse.cpp
urbiscale_SOURCES = urbiscale.cpp
urbiwalk_SOURCES = urbiwalk.cpp Move.cpp Move.h

urbiwalk_LDADD = $(top_builddir)/lib/jpeg-6b/libjpeg.la $(top_builddir)/src/liburbi/liburbi.la
