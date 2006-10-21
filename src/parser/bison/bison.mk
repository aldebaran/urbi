## -------------- ##
## Bison parser.  ##
## -------------- ##

# A Bison wrapper for C++.
BISONXX = $(top_builddir)/build-aux/bison++
BISONXX_IN = $(top_srcdir)/build-aux/bison++.in

parsedir = $(top_srcdir)/src/parser/bison

# We do not use Automake features here.
FROM_UGRAMMAR_Y =			\
stack.hh				\
position.hh				\
location.hh				\
ugrammar.hh				\
ugrammar.cc

BUILT_SOURCES += $(FROM_UGRAMMAR_Y)
CLEANFILES += $(FROM_UGRAMMAR_Y) ugrammar.stamp ugrammar.output
# We use a small library just to emulate Per-Object Flags.
# See the Automake documentation.  We need to pass -O0 when
# compiling ugrammar.cc with mipsel-linux-c++, to avoid this:
#
#  {standard input}: Assembler messages:
#  {standard input}:4842: Error: Branch out of range
#  {standard input}:5216: Error: Branch out of range
#  {standard input}:5395: Error: Branch out of range
#  {standard input}:5435: Error: Branch out of range
#  {standard input}:5485: Error: Branch out of range
#  {standard input}:5541: Error: Branch out of range
#  {standard input}:5569: Error: Branch out of range
#  {standard input}:5595: Error: Branch out of range
#  
noinst_LTLIBRARIES = libparser.la
nodist_libparser_la_SOURCES = $(FROM_UGRAMMAR_Y)
libparser_la_CXXFLAGS = $(AM_CXXFLAGS) $(PARSER_CXXFLAGS)
libkernel_la_LIBADD = libparser.la

# Compile the parser and save cycles.
# This code comes from "Handling Tools that Produce Many Outputs",
# from the Automake documentation.
EXTRA_DIST += $(parsedir)/ugrammar.y
ugrammar.stamp: $(parsedir)/ugrammar.y $(BISONXX_IN) $(parsedir)/bison.mk
	$(MAKE) $(AM_MAKEFLAGS) $(BISONXX)
	@rm -f $@.tmp
	@touch $@.tmp
	$(BISONXX) $(parsedir)/ugrammar.y ugrammar.cc -d -ra
	@mv -f $@.tmp $@

$(FROM_UGRAMMAR_Y): ugrammar.stamp
	@if test -f $@; then :; else \
	  rm -f ugrammar.stamp; \
	  $(MAKE) $(AM_MAKEFLAGS) ugrammar.stamp; \
	fi

## -------------- ##
## Flex Scanner.  ##
## -------------- ##

# Flex 2.5.4's C++ output does not use std:: properly.  This is a Perl
# regexp of entities to prefix with std::.
flex_nonstd = 'cin|cout|cerr|[io]stream'

FROM_UTOKEN_L =			\
utoken.cc

BUILT_SOURCES += $(FROM_UTOKEN_L)
CLEANFILES += $(FROM_UTOKEN_L) utoken.stamp
dist_libkernel_la_SOURCES += $(parsedir)/flex-lexer.hh
nodist_libkernel_la_SOURCES = $(FROM_UTOKEN_L)

EXTRA_DIST += $(parsedir)/utoken.l
utoken.stamp: $(parsedir)/utoken.l $(parsedir)/bison.mk
	@rm -f $@.tmp
	@touch $@.tmp
	$(FLEX) -+ -outoken.cc $(parsedir)/utoken.l
	perl -pi						\
	     -e 's,<FlexLexer.h>,"parser/bison/flex-lexer.hh",;'\
	     -e 's/class istream;/#include <iostream>/;'	\
	     -e 's/([	 &])('$(flex_nonstd)')/$$1std::$$2/g;'	\
	     utoken.cc
	@mv -f $@.tmp $@

$(FROM_UTOKEN_L): utoken.stamp
	@if test -f $@; then :; else \
	  rm -f utoken.stamp; \
	  $(MAKE) $(AM_MAKEFLAGS) utoken.stamp; \
	fi


# Kludge to install userver.h.
kernelinclude_HEADERS += 		\
stack.hh				\
position.hh				\
location.hh				\
ugrammar.hh

bisonincludedir = $(kernelincludedir)/parser/bison
bisoninclude_HEADERS = \
$(parsedir)/flex-lexer.hh
