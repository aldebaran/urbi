## -------------- ##
## Bison parser.  ##
## -------------- ##

# A Bison wrapper for C++.
BISONXX = $(top_builddir)/build-aux/bison++
BISONXX_IN = $(top_srcdir)/build-aux/bison++.in
$(BISONXX): $(BISONXX_IN)
	cd $(top_builddir) && $(MAKE) $(AM_MAKEFLAGS) build-aux/bison++

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
nodist_libkernel_la_SOURCES = $(FROM_UGRAMMAR_Y)


# Compile the parser and save cycles.
# This code comes from "Handling Tools that Produce Many Outputs",
# from the Automake documentation.
EXTRA_DIST += $(parsedir)/ugrammar.y
ugrammar_deps = $(BISONXX_IN) $(parsedir)/bison.mk $(top_srcdir)/build-aux/output-to-html
ugrammar.stamp: $(parsedir)/ugrammar.y $(ugrammar_deps)
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
nodist_libkernel_la_SOURCES += $(FROM_UTOKEN_L)

EXTRA_DIST += $(parsedir)/utoken.l
utoken.stamp: $(parsedir)/utoken.l $(parsedir)/bison.mk
	@rm -f $@.tmp
	@touch $@.tmp
# -s to disable the default rule (ECHO).
	$(FLEX) -s -+ -outoken.cc $(parsedir)/utoken.l
	perl -pi						\
	     -e 's,<FlexLexer.h>,"parser/bison/flex-lexer.hh",;'\
	     -e 's/class istream;/#include <iostream>/;'	\
	     -e 's/([	 &])('$(flex_nonstd)')/$$1std::$$2/g;'	\
	     -e 's/# *include *<unistd.h>/#include "config.h"\n#ifndef WIN32\n$$&\n#endif/;'	\
	     utoken.cc
## For some reason, on Windows machine perl does not remove the back up file.
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

bisonincludedir = $(kernelincludedir)/parser/bison
bisoninclude_HEADERS = \
$(parsedir)/flex-lexer.hh
