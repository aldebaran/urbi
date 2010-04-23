## Copyright (C) 2008-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

dist_libuobject@LIBSFX@_la_SOURCES +=			\
  parser/fwd.hh					\
  parser/is-keyword.hh				\
  parser/is-keyword.cc				\
  parser/metavar-map.hh				\
  parser/metavar-map.hxx			\
  parser/parse.hh				\
  parser/parse.cc				\
  parser/prescan.hh				\
  parser/prescan.cc				\
  parser/utoken.hh				\
  parser/parse-result.hh			\
  parser/parse-result.hxx			\
  parser/parse-result.cc			\
  parser/parser-impl.hh				\
  parser/parser-impl.hxx			\
  parser/parser-impl.cc				\
  parser/parser-utils.hh			\
  parser/parser-utils.cc			\
  parser/transform.hh				\
  parser/transform.cc				\
  parser/uparser.hh				\
  parser/uparser.cc


## --------------------- ##
## bison/flex wrappers.  ##
## --------------------- ##

# A Bison wrapper for C++.
BISONXX = $(top_builddir)/build-aux/bin/bison++
BISONXX_IN = $(top_srcdir)/build-aux/bin/bison++.in
$(BISONXX): $(BISONXX_IN)
	$(MAKE) -C $(top_builddir) $(AM_MAKEFLAGS) build-aux/bison++

# A Flex wrapper for C++.
FLEXXX = $(top_builddir)/build-aux/bin/flex++
FLEXXX_IN = $(top_srcdir)/build-aux/bin/flex++.in
$(FLEXXX): $(FLEXXX_IN)
	$(MAKE) -C $(top_builddir) $(AM_MAKEFLAGS) build-aux/bin/flex++

## From flex.info.
##
## The default setting is `-Cem', which specifies that `flex'
## should generate equivalence classes and meta-equivalence
## classes.  This setting provides the highest degree of table
## compression.
##
##  slowest & smallest
##        -Cem
##        -Cm
##        -Ce
##        -C
##        -C{f,F}e
##        -C{f,F}
##        -C{f,F}a
##  fastest & largest
##
## `-Cfe' is often a good compromise between speed and size for
## production scanners.
FLEXXXFLAGS =
if COMPILATION_MODE_SPACE
  FLEXXXFLAGS += -Cem
else !COMPILATION_MODE_SPACE
if COMPILATION_MODE_SPEED
   FLEXXXFLAGS += -Cfa
endif COMPILATION_MODE_SPEED
endif !COMPILATION_MODE_SPACE
if COMPILATION_MODE_DEBUG
  FLEXXXFLAGS += --debug
endif

## -------------- ##
## Bison parser.  ##
## -------------- ##

# We do not use Automake features here.
SOURCES_FROM_UGRAMMAR_Y =			\
  parser/stack.hh				\
  parser/position.hh				\
  parser/location.hh				\
  parser/ugrammar.hh				\
  parser/ugrammar.cc
nodist_libuobject@LIBSFX@_la_SOURCES += $(SOURCES_FROM_UGRAMMAR_Y)

DATA_FROM_UGRAMMAR_Y = 				\
  parser/ugrammar.html				\
  parser/ugrammar.output			\
  parser/ugrammar.stamp				\
  parser/ugrammar.xml
nodist_noinst_DATA += $(DATA_FROM_UGRAMMAR_Y)

FROM_UGRAMMAR_Y = $(SOURCES_FROM_UGRAMMAR_Y) $(DATA_FROM_UGRAMMAR_Y)
MAINTAINERCLEANFILES += $(FROM_UGRAMMAR_Y)

EXTRA_DIST += parser/ugrammar.y
precompiled_symbols_hh_deps += parser/ugrammar.y
ugrammar_deps =					\
  $(BISONXX_IN)					\
  $(top_srcdir)/build-aux/bin/fuse-switch	\
  parser/local.mk				\
  $(wildcard $(top_srcdir)/bison/data/*.c)	\
  $(wildcard $(top_srcdir)/bison/data/*.cc)	\
  $(wildcard $(top_srcdir)/bison/data/*.m4)


# Bison options.  Use space-expensive features only when not in space
# compilation mode.
AM_BISONFLAGS = --defines --report=all
if !COMPILATION_MODE_SPACE
  AM_BISONFLAGS += -Dparse.error=verbose -Dlr.default-reductions=consistent
endif
if COMPILATION_MODE_DEBUG
  AM_BISONFLAGS += -Dparse.assert -Dparse.trace
endif
# Compile the parser and save cycles.
# This code comes from "Handling Tools that Produce Many Outputs",
# from the Automake documentation.
parser/ugrammar.stamp: parser/ugrammar.y $(ugrammar_deps)
	rm -f $@ $@.tmp parser/ugrammar-pruned.y
	echo '$@ rebuilt because of: $?' >$@.tmp
	$(MAKE) $(BISONXX)
	$(MAKE) -C $(top_builddir)/bison MAKEFLAGS=
	$(PRUNE_FOR_SPACE) - <$< >parser/ugrammar-pruned.y
	chmod a-w parser/ugrammar-pruned.y
	$(BISONXX) parser/ugrammar-pruned.y parser/ugrammar.cc \
	  $(AM_BISONFLAGS) $(BISONFLAGS)
	mv -f $@.tmp $@

# Not $(FROM_UGRAMMAR_Y) since it contains ugrammar.stamp too.
# See Automake's documentation for the whole story.
#
# The GNU Make documentation seems clear about the fact that when the
# target (say parser/stack.hh) exists but is outdated, then the "VPATH
# is ignored" and "$@" denotes only "parser/stack.hh", not including
# the VPATH prefix.  Yet GNU Make 3.81 on OSX and Linux does replace
# this "$@" with "$(srcdir)/parser/stack.hh".  But for some reason we
# have not understood yet, on Windows it does not.  That's why we have
# to test twice, with and without $(srcdir).
#
# Alternatively we could use GPATH, but I, Akim, have no experience
# with it, and I don't know if we wouldn't have troubles elsewhere
# (other rules of this Makefile which expect VPATH behavior, not
# GPATH).
$(SOURCES_FROM_UGRAMMAR_Y): parser/ugrammar.stamp
	@if test ! -f $@; then					\
	  trap 'rm -rf parser/ugrammar.{lock,stamp}' 1 2 13 15;	\
          if mkdir parser/ugrammar.lock 2>/dev/null; then	\
	    rm -f parser/ugrammar.stamp;			\
	    $(MAKE) $(AM_MAKEFLAGS) parser/ugrammar.stamp;	\
	    result=$$?;						\
	    rm -rf parser/ugrammar.lock;			\
	    exit $$result;					\
	  else							\
	    while test -d parser/ugrammar.lock; do		\
	      sleep 1;						\
	    done;						\
	    test -f parser/ugrammar.stamp;			\
	  fi;							\
	fi

# We tried several times to run make from ast/ to build position.hh
# and location.hh.  Unfortunately, because of different, but
# equivalent, paths, BSD Make was unable to build them.  The following
# hook is here to address this.
.PHONY: generate-parser
generate-parser: $(FROM_UGRAMMAR_Y)


## ------------------------------------ ##
## Keyword list for lstlistings/emacs.  ##
## ------------------------------------ ##

EXTRA_DIST += parser/keywords

.PHONY: listings emacs
listings: parser/utoken.l
	KEYWORDS_MODE=$@ $(srcdir)/parser/keywords $<

emacs: parser/utoken.l
	KEYWORDS_MODE=$@ $(srcdir)/parser/keywords $<

nodist_libuobject@LIBSFX@_la_SOURCES += parser/keywords.hh
parser/keywords.hh: parser/utoken.l
	rm -f $@
	KEYWORDS_MODE=c++ $(srcdir)/parser/keywords $< >$@.tmp
	mv $@.tmp $@

## -------------- ##
## Flex Scanner.  ##
## -------------- ##

FROM_UTOKEN_L =			\
  parser/utoken.cc

CLEANFILES += parser/utoken.stamp
dist_libuobject@LIBSFX@_la_SOURCES += parser/flex-lexer.hh
nodist_libuobject@LIBSFX@_la_SOURCES += $(FROM_UTOKEN_L)

EXTRA_DIST += parser/utoken.l
utoken_deps = $(FLEXXX_IN) parser/local.mk
parser/utoken.stamp: parser/utoken.l $(utoken_deps)
	$(MAKE) $(AM_MAKEFLAGS) $(FLEXXX)
	@rm -f $@.tmp
	@touch $@.tmp
	$(PRUNE_FOR_SPACE) - <$< >parser/utoken-pruned.l
	$(FLEXXX) parser/utoken-pruned.l parser/utoken.cc $(FLEXXXFLAGS)
	@mv -f $@.tmp $@

$(FROM_UTOKEN_L): parser/utoken.stamp
	@if test ! -f $@; then			\
	  rm -f $<;				\
	  $(MAKE) $(AM_MAKEFLAGS) $<;		\
	fi
