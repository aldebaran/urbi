AM_CXXFLAGS += $(WARNING_CXXFLAGS)

## -------------- ##
## Bison parser.  ##
## -------------- ##

# A Bison wrapper for C++.
BISONXX = $(top_builddir)/build-aux/bison++
BISONXX_IN = $(top_srcdir)/build-aux/bison++.in

parsedir = $(srcdir)/parser/bison
# We do not use Automake features here.
FROM_UGRAMMAR_Y =				\
$(parsedir)/stack.hh				\
$(parsedir)/position.hh				\
$(parsedir)/location.hh				\
$(parsedir)/ugrammar.hh				\
$(parsedir)/ugrammar.cc

BUILT_SOURCES += $(FROM_UGRAMMAR_Y)
MAINTAINERCLEANFILES = $(FROM_UGRAMMAR_Y)
dist_libkernel_la_SOURCES += $(FROM_UGRAMMAR_Y)

# Compile the parser and save cycles.
# This code comes from "Handling Tools that Produce Many Outputs",
# from the Automake documentation.
EXTRA_DIST += $(parsedir)/ugrammar.stamp $(parsedir)/ugrammar.y
$(parsedir)/ugrammar.stamp: $(parsedir)/ugrammar.y $(BISONXX_IN)
	$(MAKE) $(AM_MAKEFLAGS) $(BISONXX)
	@rm -f ugrammar.tmp
	@touch ugrammar.tmp
	$(BISONXX) $(parsedir) ugrammar.y ugrammar.cc -d -ra
	@mv -f ugrammar.tmp $@

$(FROM_UGRAMMAR_Y): $(parsedir)/ugrammar.stamp
	@if test -f $@; then :; else \
	  rm -f $(parsedir)/ugrammar.stamp; \
	  $(MAKE) $(AM_MAKEFLAGS) $(parsedir)/ugrammar.stamp; \
	fi

## -------------- ##
## Flex Scanner.  ##
## -------------- ##

# Flex 2.5.4 is quite old, and its C++ output does not use std::
# properly.  This is the list of entities which must be prefixed by
# std::.
flex_nonstd = 'cin|cout|cerr|istream'
EXTRA_DIST += $(parsedir)/utoken.l
dist_libkernel_la_SOURCES += $(parsedir)/FlexLexer.h $(parsedir)/utoken.cc
$(parsedir)/utoken.cc: $(parsedir)/utoken.l
	$(FLEX) -+ -o$@.tmp $(parsedir)/utoken.l
	perl -pi $@.tmp						\
	     -e 's,<FlexLexer.h>,"parser/bison/FlexLexer.h",;'	\
	     -e 's/class istream;/#include <istream>/;'		\
	     -e 's/([	 &])('$(flex_nonstd)')/$$1std::$$2/g;'	\
	     -e 's,lex.cc,utoken.cc,g;'
	mv $@.tmp $@

# ifeq ($(OS),aibo)
# parser/bison/ugrammar.mips.o: parser/bison/ugrammar.cc
# 	echo "special compile for Aibo: no optim"
# 	$(CC) $(CPPFLAGS) -Wno-deprecated $(OPTIM) -O0 -o $@ -c $<
# endif




# Kludge to install userver.h.
bisondir = $(kernelincludedir)/parser/bison
bison_HEADERS = 				\
$(parsedir)/stack.hh				\
$(parsedir)/position.hh				\
$(parsedir)/location.hh				\
$(parsedir)/ugrammar.hh				\
$(parsedir)/FlexLexer.h
