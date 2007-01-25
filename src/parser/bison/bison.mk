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
nodist_libkernel_la_SOURCES = $(FROM_UGRAMMAR_Y)

## ------------- ##
## ugrammar.lo.  ##
## ------------- ##

# We need to pass -O0 when compiling ugrammar.cc with mipsel-linux-c++,
# to avoid this:
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
# This is an edited copy of Automake's compilation rule so that we can
# insert $(PARSER_CXXFLAGS) *after* $(CXXFLAGS) to override -O2.
# Using foo_CXXFLAGS does not suffice, since CXXFLAGS, a user
# variable, appears last.
ugrammar.lo: ugrammar.cc
@am__fastdepCXX_TRUE@	if $(LTCXXCOMPILE) $(PARSER_CXXFLAGS) -MT ugrammar.lo -MD -MP -MF "$(DEPDIR)/ugrammar.Tpo" -c -o ugrammar.lo `test -f 'ugrammar.cc' || echo '$(srcdir)/'`ugrammar.cc; \
@am__fastdepCXX_TRUE@	then mv -f "$(DEPDIR)/ugrammar.Tpo" "$(DEPDIR)/ugrammar.Plo"; else rm -f "$(DEPDIR)/ugrammar.Tpo"; exit 1; fi
@AMDEP_TRUE@@am__fastdepCXX_FALSE@	source='ugrammar.cc' object='ugrammar.lo' libtool=yes @AMDEPBACKSLASH@
@AMDEP_TRUE@@am__fastdepCXX_FALSE@	DEPDIR=$(DEPDIR) $(CXXDEPMODE) $(depcomp) @AMDEPBACKSLASH@
@am__fastdepCXX_FALSE@	$(LTCXXCOMPILE) $(PARSER_CXXFLAGS) -c -o ugrammar.lo `test -f 'ugrammar.cc' || echo '$(srcdir)/'`ugrammar.cc

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
