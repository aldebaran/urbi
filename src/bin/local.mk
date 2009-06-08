## ---------- ##
## ast-dump.  ##
## ---------- ##

EXTRA_PROGRAMS = bin/ast-dump
dist_bin_ast_dump_SOURCES = bin/ast-dump.cc
bin_ast_dump_CPPFLAGS = $(libuobject_la_CPPFLAGS)
bin_ast_dump_LDADD    =	libuobject.la

if BUILD_PROGRAMS
if !WIN32
noinst_PROGRAMS = bin/ast-dump
endif !WIN32
endif BUILD_PROGRAMS

## ------ ##
## urbi.  ##
## ------ ##

bin_PROGRAMS += bin/urbi
bin_urbi_SOURCES = bin/urbi.cc
bin_urbi_LDADD = $(LIBPORT_LIBS)
