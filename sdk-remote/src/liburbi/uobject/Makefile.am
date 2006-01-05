INCLUDES = -I$(top_srcdir)/src/liburbi
AM_CXXFLAGS = -pthread -Wno-deprecated
lib_LIBRARIES = libuobject.a
libuobject_a_SOURCES = uobject.cc uobject.h uobjectdata.cc uobjectdata.h uvar.cc usharedexternal.h

uobject.cc: uobject.h 
uobject.h: uobject.template.h
	./template_autogen.pl uobject.template.h > uobject.h
