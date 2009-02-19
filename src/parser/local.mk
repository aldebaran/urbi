dist_libuobject_la_SOURCES +=			\
  parser/ast-factory.hh				\
  parser/ast-factory.hxx			\
  parser/ast-factory.cc				\
  parser/event-match.hh				\
  parser/fwd.hh					\
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
BISONXX = $(top_builddir)/build-aux/bison++
BISONXX_IN = $(top_srcdir)/build-aux/bison++.in
$(BISONXX): $(BISONXX_IN)
	$(MAKE) -C $(top_builddir) $(AM_MAKEFLAGS) build-aux/bison++

# A Flex wrapper for C++.
FLEXXX = $(top_builddir)/build-aux/flex++
FLEXXX_IN = $(top_srcdir)/build-aux/flex++.in
$(FLEXXX): $(FLEXXX_IN)
	$(MAKE) -C $(top_builddir) $(AM_MAKEFLAGS) build-aux/flex++


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
BUILT_SOURCES += $(SOURCES_FROM_UGRAMMAR_Y)
dist_libuobject_la_SOURCES += $(SOURCES_FROM_UGRAMMAR_Y)

DATA_FROM_UGRAMMAR_Y = 				\
  parser/ugrammar.html				\
  parser/ugrammar.output			\
  parser/ugrammar.stamp				\
  parser/ugrammar.xml
dist_noinst_DATA += $(DATA_FROM_UGRAMMAR_Y)

FROM_UGRAMMAR_Y = $(SOURCES_FROM_UGRAMMAR_Y) $(DATA_FROM_UGRAMMAR_Y)
MAINTAINERCLEANFILES += $(FROM_UGRAMMAR_Y)

# Compile the parser and save cycles.
# This code comes from "Handling Tools that Produce Many Outputs",
# from the Automake documentation.
EXTRA_DIST += parser/ugrammar.y
precompiled_symbols_hh_deps += parser/ugrammar.y
ugrammar_deps =					\
  $(BISONXX_IN)					\
  $(top_srcdir)/build-aux/fuse-switch		\
  parser/local.mk				\
  $(wildcard $(top_builddir)/bison/data/*.c)	\
  $(wildcard $(top_builddir)/bison/data/*.cc)	\
  $(wildcard $(top_builddir)/bison/data/*.m4)

$(srcdir)/parser/ugrammar.stamp: parser/ugrammar.y $(ugrammar_deps)
	rm -f $@ $@.tmp
	echo '$@ rebuilt because of: $?' >$@.tmp
	$(MAKE) $(BISONXX)
	$(MAKE) -C $(top_builddir)/bison MAKEFLAGS=
	$(BISONXX) $< $(srcdir)/parser/ugrammar.cc -d -ra $(BISON_FLAGS)
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
$(SOURCES_FROM_UGRAMMAR_Y): $(srcdir)/parser/ugrammar.stamp
	@if test ! -f $@ && test ! -f $(srcdir)/$@; then		  \
	  trap 'rm -rf $(srcdir)/parser/ugrammar.{lock,stamp}' 1 2 13 15; \
          if mkdir $(srcdir)/parser/ugrammar.lock 2>/dev/null; then	  \
	    rm -f $(srcdir)/parser/ugrammar.stamp;			  \
	    $(MAKE) $(AM_MAKEFLAGS) parser/ugrammar.stamp;		  \
	    result=$$?;							  \
	    rm -rf $(srcdir)/parser/ugrammar.lock;			  \
	    exit $$result;						  \
	  else								  \
	    while test -d $(srcdir)/parser/ugrammar.lock; do		  \
	      sleep 1;							  \
	    done;							  \
	    test -f $(srcdir)/parser/ugrammar.stamp;			  \
	  fi;								  \
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

.PHONY: listings emacs
listings: parser/utoken.l
	perl -ne 'BEGIN { use Text::Wrap; }' \
	     -e '/^"(\w+)"/ && push @k, $$1;' \
	     -e 'END { print wrap ("    ", "    ", join (", ", map { s/_/\\_/g; $$_ } sort @k)), "\n" };' $<

emacs: parser/utoken.l
	perl -ne 'BEGIN { use Text::Wrap; }' \
	     -e '/^"(\w+)"/ && push @k, $$1;' \
	     -e 'END { print wrap ("            ", "            ", join (" ", map { "\"$$_\"" } sort @k)), "\n" };' $<


## -------------- ##
## Flex Scanner.  ##
## -------------- ##

FROM_UTOKEN_L =			\
  parser/utoken.cc

BUILT_SOURCES += $(FROM_UTOKEN_L)
CLEANFILES += $(FROM_UTOKEN_L) parser/utoken.stamp
dist_libuobject_la_SOURCES += parser/flex-lexer.hh
nodist_libuobject_la_SOURCES += $(FROM_UTOKEN_L)

EXTRA_DIST += parser/utoken.l
utoken_deps = $(FLEXXX_IN) parser/local.mk
parser/utoken.stamp: parser/utoken.l $(utoken_deps)
	$(MAKE) $(AM_MAKEFLAGS) $(FLEXXX)
	@rm -f $@.tmp
	@touch $@.tmp
	$(FLEXXX) $< parser/utoken.cc
	@mv -f $@.tmp $@

$(FROM_UTOKEN_L): parser/utoken.stamp
	@if test ! -f $@; then			\
	  rm -f $<;				\
	  $(MAKE) $(AM_MAKEFLAGS) $<;		\
	fi
