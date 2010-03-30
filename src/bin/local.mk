## ---------- ##
## ast-dump.  ##
## ---------- ##

EXTRA_PROGRAMS = bin/ast-dump bin/urbi-parse bin/urbi-pp bin/urbi-compile

if BUILD_PROGRAMS
if !WIN32
noinst_PROGRAMS = bin/ast-dump
if ENABLE_SERIALIZATION
bin_PROGRAMS += bin/urbi-parse bin/urbi-pp bin/urbi-compile
endif
endif !WIN32
endif BUILD_PROGRAMS

# ast-dump.
bin_ast_dump_CPPFLAGS = $(libuobject@LIBSFX@_la_CPPFLAGS)
bin_ast_dump_LDADD = libuobject$(LIBSFX).la

# urbi.
bin_PROGRAMS += bin/urbi
bin_urbi_SOURCES = bin/urbi.cc ../sdk-remote/src/bin/urbi-root.cc
bin_urbi_CPPFLAGS = -DURBI_ROOT_NOT_DLL $(AM_CPPFLAGS)
bin_urbi_LDADD = $(LIBPORT_LIBS)

if STATIC_BUILD
bin_urbi_LDADD += $(top_builddir)/sdk-remote/src/libuvalue/libuvalue.la libuobject.la
endif

# urbi-parse.
bin_urbi_parse_CPPFLAGS = $(libuobject@LIBSFX@_la_CPPFLAGS)
bin_urbi_parse_LDADD = libuobject$(LIBSFX).la

# urbi-pp.
bin_urbi_pp_CPPFLAGS = $(libuobject@LIBSFX@_la_CPPFLAGS)
bin_urbi_pp_LDADD = libuobject$(LIBSFX).la

# urbi-compile.
bin_urbi_compile_CPPFLAGS = $(libuobject@LIBSFX@_la_CPPFLAGS)
bin_urbi_compile_LDADD = libuobject$(LIBSFX).la
