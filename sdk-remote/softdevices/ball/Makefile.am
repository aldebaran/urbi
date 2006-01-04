INCLUDES = -I$(top_srcdir)/src/liburbi -I$(top_srcdir)/src/liburbi/uobject
AM_LDFLAGS = -L$(top_srcdir)/src/liburbi -L$(top_srcdir)/src/liburbi/uobject
AM_CXXFLAGS = -pthread

bin_PROGRAMS = ball

ball_SOURCES = ball.cc ball.h ../umain.cc
ball_LDADD = -lurbi -luobject
