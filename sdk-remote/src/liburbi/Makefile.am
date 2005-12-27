INCLUDES = -I$(top_srcdir)/lib/jpeg-6b
AM_LDFLAGS = -L$(top_srcdir)/lib/jpeg-6b
AM_CXXFLAGS = -pthread
lib_LIBRARIES = liburbi.a
include_HEADERS = uclient.h uabstractclient.h usyncclient.h

liburbi.a: uabstractclient.o uclient.o usyncclient.o
	rm -f liburbi.a
	cp $(top_srcdir)/lib/jpeg-6b/libjpeg.a ./liburbi.a
	ar r liburbi.a uabstractclient.o uclient.o usyncclient.o
	ranlib liburbi.a

liburbi_a_SOURCES = uabstractclient.h uclient.h uabstractclient.cpp uclient.cpp 
liburbi_a_LIBADD = $(top_srcdir)/lib/jpeg-6b/libjpeg.a

if OPENR
SUBDIRS = openr
endif OPENR 
