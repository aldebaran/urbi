dist_libkernel_la_SOURCES +=					\
parser/uparser.hh parser/uparser.cc


## -------------- ##
## Bison parser.  ##
## -------------- ##

# A Bison wrapper for C++.
BISONXX = $(top_builddir)/libport/build-aux/bison++
BISONXX_IN = $(top_srcdir)/libport/build-aux/bison++.in
$(BISONXX): $(BISONXX_IN)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) libport/build-aux/bison++

parser_dir = $(top_srcdir)/src/parser

# We do not use Automake features here.
FROM_UGRAMMAR_Y =			\
stack.hh				\
position.hh				\
location.hh				\
ugrammar.hh				\
ugrammar.cc

BUILT_SOURCES += $(FROM_UGRAMMAR_Y)
CLEANFILES += $(FROM_UGRAMMAR_Y)
CLEANFILES += ugrammar.{html,output,stamp,xml}
nodist_libkernel_la_SOURCES += $(FROM_UGRAMMAR_Y)


# Compile the parser and save cycles.
# This code comes from "Handling Tools that Produce Many Outputs",
# from the Automake documentation.
EXTRA_DIST += $(parser_dir)/ugrammar.y
ugrammar_deps = $(BISONXX_IN) $(parser_dir)/parser.mk
ugrammar.stamp: $(parser_dir)/ugrammar.y $(ugrammar_deps)
	$(MAKE) $(AM_MAKEFLAGS) $(BISONXX)
	@rm -f $@.tmp
	@touch $@.tmp
	$(BISONXX) $(parser_dir)/ugrammar.y ugrammar.cc -d -ra
	@mv -f $@.tmp $@

$(FROM_UGRAMMAR_Y): ugrammar.stamp
	@if test -f $@; then :; else \
	  rm -f ugrammar.stamp; \
	  $(MAKE) $(AM_MAKEFLAGS) ugrammar.stamp; \
	fi

# We tried several times to run make from ast/ to build position.hh
# and location.hh.  Unfortunately, because of different, but
# equivalent, paths, BSD Make was unable to build them.  The following
# hook is here to address this.
.PHONY: generate-parser
generate-parser: $(FROM_UGRAMMAR_Y)


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
dist_libkernel_la_SOURCES += $(parser_dir)/flex-lexer.hh
nodist_libkernel_la_SOURCES += $(FROM_UTOKEN_L)

EXTRA_DIST += $(parser_dir)/utoken.l
utoken.stamp: $(parser_dir)/utoken.l $(parser_dir)/parser.mk
	@rm -f $@.tmp
	@touch $@.tmp
# -s to disable the default rule (ECHO).
	$(FLEX) -s -+ -outoken.cc $(parser_dir)/utoken.l
	perl -pi						\
	     -e 's,<FlexLexer.h>,"parser/flex-lexer.hh",;'	\
	     -e 's/class istream;/#include <iostream>/;'	\
	     -e 's/([	 &])('$(flex_nonstd)')/$$1std::$$2/g;'	\
	     -e 's,# *include *<unistd.h>,#include "sdk/config.h"\n#ifndef WIN32\n$$&\n#endif,'	\
	     utoken.cc
## For some reason, on Windows perl does not remove the back up file.
	rm -f utoken.cc.bak
	@mv -f $@.tmp $@

$(FROM_UTOKEN_L): utoken.stamp
	@if test -f $@; then :; else \
	  rm -f utoken.stamp; \
	  $(MAKE) $(AM_MAKEFLAGS) utoken.stamp; \
	fi


# Kludge to install userver.hh.
kernelinclude_HEADERS += 		\
stack.hh				\
position.hh				\
location.hh				\
ugrammar.hh

nobase_kernelinclude_HEADERS = 		\
parser/flex-lexer.hh
