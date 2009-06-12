## ---------- ##
## ast-dump.  ##
## ---------- ##

EXTRA_PROGRAMS = bin/ast-dump bin/urbi-parse bin/urbi-pp bin/urbi-compile

if BUILD_PROGRAMS
if !WIN32
noinst_PROGRAMS = bin/ast-dump
bin_PROGRAMS += bin/urbi-parse bin/urbi-pp bin/urbi-compile
endif !WIN32
endif BUILD_PROGRAMS

# ast-dump.
dist_bin_ast_dump_SOURCES = bin/ast-dump.cc
bin_ast_dump_CPPFLAGS = $(libuobject_la_CPPFLAGS)
bin_ast_dump_LDADD = libuobject.la

# urbi.
bin_PROGRAMS += bin/urbi
bin_urbi_SOURCES = bin/urbi.cc
bin_urbi_LDADD = $(LIBPORT_LIBS)

# urbi-parse.
dist_bin_urbi_parse_SOURCES = bin/urbi-parse.cc
bin_urbi_parse_CPPFLAGS = $(libuobject_la_CPPFLAGS)
bin_urbi_parse_LDADD = libuobject.la

# urbi-pp.
dist_bin_urbi_pp_SOURCES = bin/urbi-pp.cc
bin_urbi_pp_CPPFLAGS = $(libuobject_la_CPPFLAGS)
bin_urbi_pp_LDADD = libuobject.la

# urbi-compile.
dist_bin_urbi_compile_SOURCES = bin/urbi-compile.cc
bin_urbi_compile_CPPFLAGS = $(libuobject_la_CPPFLAGS)
bin_urbi_compile_LDADD = libuobject.la
