INCLUDES = -I$(top_srcdir)/src/liburbi -I$(top_srcdir)/src/liburbi/uobject
AM_LDFLAGS = -L$(top_srcdir)/src/liburbi -L$(top_srcdir)/src/liburbi/uobject
AM_CXXFLAGS = -pthread -Wno-deprecated

bin_PROGRAMS = ball

ball_SOURCES = ball.cc ball.h ../umain.cc 
ball_DEPENDENCIES = $(top_srcdir)/src/liburbi/liburbi.a $(top_srcdir)/src/liburbi/uobject/libuobject.a
ball_LDADD = -lurbi -luobject


