AM_CXXFLAGS += $(WARNING_CXXFLAGS)

## -------------- ##
## Bison parser.  ##
## -------------- ##

# A Bison wrapper for C++.
BISONXX = $(top_builddir)/build-aux/bison++
BISONXX_IN = $(top_srcdir)/build-aux/bison++.in

parsedir = $(top_srcdir)/parser/bison

# We do not use Automake features here.
FROM_UGRAMMAR_Y =			\
stack.hh				\
position.hh				\
location.hh				\
ugrammar.hh				\
ugrammar.cc

BUILT_SOURCES += $(FROM_UGRAMMAR_Y)
CLEANFILES += $(FROM_UGRAMMAR_Y) ugrammar.stamp ugrammar.output
nodist_libkernel_la_SOURCES = $(FROM_UGRAMMAR_Y)

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

# Flex 2.5.4 is quite old, and its C++ output does not use std::
# properly.  This is the list of entities which must be prefixed by
# std::.
flex_nonstd = 'cin|cout|cerr|[io]stream'

FROM_UTOKEN_L =			\
utoken.cc

BUILT_SOURCES += $(FROM_UTOKEN_L)
CLEANFILES += $(FROM_UTOKEN_L) utoken.stamp
dist_libkernel_la_SOURCES += $(parsedir)/FlexLexer.h
nodist_libkernel_la_SOURCES += $(FROM_UTOKEN_L)

EXTRA_DIST += $(parsedir)/utoken.l
utoken.stamp: $(parsedir)/utoken.l $(parsedir)/bison.mk
	@rm -f $@.tmp
	@touch $@.tmp
	$(FLEX) -+ -outoken.cc $(parsedir)/utoken.l
	perl -pi						\
	     -e 's,<FlexLexer.h>,"parser/bison/FlexLexer.h",;'	\
	     -e 's/class istream;/#include <iostream>/;'	\
	     -e 's/([	 &])('$(flex_nonstd)')/$$1std::$$2/g;'	\
	     utoken.cc
	@mv -f $@.tmp $@

$(FROM_UTOKEN_L): utoken.stamp
	@if test -f $@; then :; else \
	  rm -f utoken.stamp; \
	  $(MAKE) $(AM_MAKEFLAGS) utoken.stamp; \
	fi

# ifeq ($(OS),aibo)
# parser/bison/ugrammar.mips.o: parser/bison/ugrammar.cc
# 	echo "special compile for Aibo: no optim"
# 	$(CC) $(CPPFLAGS) -Wno-deprecated $(OPTIM) -O0 -o $@ -c $<
# endif




# Kludge to install userver.h.
bisondir = $(kernelincludedir)/parser/bison
bison_HEADERS = 			\
stack.hh				\
position.hh				\
location.hh				\
ugrammar.hh				\
$(parsedir)/FlexLexer.h
