INCLUDES = -I$(top_srcdir)/src/liburbi
AM_LDFLAGS = -L$(top_srcdir)/src/liburbi

AM_CXXFLAGS = -pthread


bin_PROGRAMS = urbicycle urbireverse urbiwalk urbiscale

urbicycle_SOURCES = urbicycle.cpp

urbireverse_SOURCES = urbireverse.cpp

urbiscale_SOURCES = urbiscale.cpp

urbiwalk_SOURCES = urbiwalk.cpp Move.cpp Move.h
urbiwalk_LDADD =  -lurbi

