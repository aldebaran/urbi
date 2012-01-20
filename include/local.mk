## Copyright (C) 2006-2012, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## -------------------------- ##
## Installed kernel headers.  ##
## -------------------------- ##

## Handling INSTALL_KERNEL_HEADERS this way ensures that $(HEADERS)
## contains all the headers.
if INSTALL_KERNEL_HEADERS
urbi_includedir =   $(brandincludedir)/urbi
kernel_includedir = $(urbi_includedir)/kernel
object_includedir = $(urbi_includedir)/object
parser_includedir = $(urbi_includedir)/parser
runner_includedir = $(urbi_includedir)/runner
else !INSTALL_KERNEL_HEADERS
urbi_includedir =
kernel_includedir =
object_includedir =
parser_includedir =
runner_includedir =
endif !INSTALL_KERNEL_HEADERS

dist_urbi_include_HEADERS =			\
  include/urbi/sdk.hh				\
  include/urbi/sdk.hxx

dist_kernel_include_HEADERS =		        \
  include/urbi/kernel/fwd.hh			\
  include/urbi/kernel/uconnection.hh		\
  include/urbi/kernel/uconnection.hxx		\
  include/urbi/kernel/userver.hh		\
  include/urbi/kernel/userver.hxx		\
  include/urbi/kernel/utypes.hh

dist_object_include_HEADERS =	                \
  include/urbi/object/any-to-boost-function.hh	\
  include/urbi/object/barrier.hh		\
  include/urbi/object/centralized-slots.hh	\
  include/urbi/object/centralized-slots.hxx	\
  include/urbi/object/cxx-conversions.hh	\
  include/urbi/object/cxx-conversions.hxx	\
  include/urbi/object/cxx-object.hh		\
  include/urbi/object/cxx-object.hxx		\
  include/urbi/object/cxx-primitive.hh		\
  include/urbi/object/date.hh			\
  include/urbi/object/date.hxx			\
  include/urbi/object/dictionary.hh		\
  include/urbi/object/directory.hh		\
  include/urbi/object/duration.hh		\
  include/urbi/object/duration.hxx		\
  include/urbi/object/enumeration.hh		\
  include/urbi/object/equality-comparable.hh	\
  include/urbi/object/equality-comparable.hxx	\
  include/urbi/object/event.hh			\
  include/urbi/object/event-handler.hh          \
  include/urbi/object/file.hh			\
  include/urbi/object/float.hh			\
  include/urbi/object/float.hxx			\
  include/urbi/object/fwd.hh			\
  include/urbi/object/global.hh			\
  include/urbi/object/hash.hh			\
  include/urbi/object/job.hh			\
  include/urbi/object/list.hh			\
  include/urbi/object/lobby.hh			\
  include/urbi/object/lobby.hxx			\
  include/urbi/object/location.hh		\
  include/urbi/object/location.hxx		\
  include/urbi/object/object.hh			\
  include/urbi/object/object.hxx		\
  include/urbi/object/path.hh			\
  include/urbi/object/position.hh		\
  include/urbi/object/position.hxx		\
  include/urbi/object/primitive.hh		\
  include/urbi/object/slot.hh			\
  include/urbi/object/slot.hxx			\
  include/urbi/object/string.hh			\
  include/urbi/object/symbols.hh		\
  include/urbi/object/tag.hh			\
  include/urbi/object/tag.hxx                   \
  include/urbi/object/urbi-exception.hh         \
  include/urbi/object/urbi-exception.hxx

nodist_parser_include_HEADERS =                 \
  include/urbi/parser/location.hh               \
  include/urbi/parser/position.hh

dist_runner_include_HEADERS =			\
  include/urbi/runner/raise.hh


## ------------------------ ##
## Generated source files.  ##
## ------------------------ ##

FROM_GEN =					\
  include/urbi/object/any-to-boost-function.hxx	\
  include/urbi/object/cxx-primitive.hxx		\
  include/urbi/object/executable.hh
BUILT_SOURCES += $(FROM_GEN)

dist_object_include_HEADERS += $(FROM_GEN)
EXTRA_DIST += $(FROM_GEN:=.gen)

%: %.gen
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at)$< > $@.tmp
	$(AM_V_at)chmod a-w $@.tmp
	$(AM_V_at)$(move_if_change_run) $@.tmp $@
	$(AM_V_at)touch $@


## ---------------------- ##
## List of used symbols.  ##
## ---------------------- ##

nodist_object_include_HEADERS = 		\
  $(precompiled_symbols_hh)

# Generate this file in builddir so that a single srcdir can produce
# several builddirs with different configuration-options that may
# result in different sets of precompiled symbols.
precompiled_symbols_hh = include/urbi/object/precompiled-symbols.hh
precompiled_symbols_stamp = $(precompiled_symbols_hh:.hh=.stamp)
# filter-out generated files, and precompiled_symbols_hh itself to
# avoid circular dependencies.
precompiled_symbols_hh_sources =		\
  $(top_srcdir)/src/parser/utoken.l		\
  $(top_srcdir)/src/parser/ugrammar.y		\
  $(filter-out $(precompiled_symbols_hh)	\
               $(FROM_UGRAMMAR_Y)		\
               $(FROM_UTOKEN_L)			\
	       parser/keywords.hh		\
	       ast/ignores,			\
        $(call ls_files,			\
           src/*.hh		\
           src/*.hxx		\
           src/*.cc))
EXTRA_DIST += $(precompiled_symbols_hh).gen

$(precompiled_symbols_stamp): $(precompiled_symbols_hh).gen $(precompiled_symbols_hh_sources)
	$(AM_V_GEN)mkdir -p $(@D)
	$(AM_V_at){							\
	  echo "rebuilding $(precompiled_symbols_hh) because of:";	\
	  for i in $?;							\
	  do								\
	    echo "       $$i";						\
	  done;								\
	} >$@.tmp
	$(AM_V_at)test "$(V)" != 1 || cat $@.tmp
# Don't use `mv' here so that even if we are interrupted, the file
# is still available for diff in the next run.
	$(AM_V_at)if test -f $(precompiled_symbols_hh); then	\
	  cat $(precompiled_symbols_hh);			\
	fi >$(precompiled_symbols_hh)~
	$(AM_V_at)$(srcdir)/$(precompiled_symbols_hh).gen		\
		$(filter-out %$(precompiled_symbols_hh).gen, $^)	\
		>$(precompiled_symbols_hh).tmp
	$(AM_V_at)$(move_if_change_run)			\
	  $(precompiled_symbols_hh).tmp $(precompiled_symbols_hh)
	$(AM_V_at)mv -f $@.tmp $@

$(precompiled_symbols_hh): $(precompiled_symbols_stamp)
	@if test ! -f $@; then					\
	  rm -f $(precompiled_symbols_stamp);			\
	  $(MAKE) $(AM_MAKEFLAGS) $(precompiled_symbols_stamp);	\
	fi
BUILT_SOURCES += $(precompiled_symbols_hh)


## ------------------ ##
## Maintainer check.  ##
## ------------------ ##


.PHONY maintainer-check: maintainer-check-includes

cxx_headers = \
  algorithm|deque|iomanip|iosfwd|iostream|memory|ostream|set|sstream|string|typeinfo|vector
PERL = perl
maintainer-check-includes: $(HEADERS)
	$(AM_V_GEN)! $(PERL) -n						  \
	  -e '/#\s*include\s*<(.*?)>/ and $$include{$$1} = 1;'		  \
	  -e 'END { print join ("\n", (sort (keys %include), "")) }' $^ | \
	  grep -Ev '^((boost|libport|sched|urbi)/|($(cxx_headers)))'
