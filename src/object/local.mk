## Copyright (C) 2008-2010, Gostai S.A.S.
##
## This software is provided "as is" without warranty of any kind,
## either expressed or implied, including but not limited to the
## implied warranties of fitness for a particular purpose.
##
## See the LICENSE file for more information.

## ---------------------- ##
## List of used symbols.  ##
## ---------------------- ##

nodist_libuobject@LIBSFX@_la_SOURCES += 		\
  $(precompiled_symbols_hh)

# Generate this file in builddir so that a single srcdir can produce
# several builddirs with different configuration-options that may
# result in different sets of precompiled symbols.
precompiled_symbols_hh = object/precompiled-symbols.hh
precompiled_symbols_stamp = $(precompiled_symbols_hh:.hh=.stamp)
# filter-out generated files, and precompiled_symbols_hh itself to
# avoid circular dependencies.
precompiled_symbols_hh_sources =		\
  parser/utoken.l				\
  parser/ugrammar.y				\
  $(filter-out $(precompiled_symbols_hh)	\
               $(FROM_UGRAMMAR_Y)		\
               $(FROM_UTOKEN_L)			\
	       ast/ignores,			\
        $(dist_libuobject@LIBSFX@_la_SOURCES))
EXTRA_DIST += object/symbols-generate.pl

$(precompiled_symbols_stamp): object/symbols-generate.pl $(precompiled_symbols_hh_sources)
	@rm -f $@.tmp
	@echo "rebuilding $(precompiled_symbols_hh) because of:"
	@for i in $?;				\
	do					\
	  echo "       $$i";			\
	done
	@touch $@.tmp
# Don't use `mv' here so that even if we are interrupted, the file
# is still available for diff in the next run.
	@if test -f $(precompiled_symbols_hh); then	\
	  cat $(precompiled_symbols_hh);		\
	fi >$(precompiled_symbols_hh)~
	@(cd $(srcdir) &&				\
	 ./object/symbols-generate.pl			\
		$(precompiled_symbols_hh_sources))	\
		>$(precompiled_symbols_hh).tmp
	@diff -u $(precompiled_symbols_hh)~ $(precompiled_symbols_hh).tmp || true
	@$(move_if_change)						\
	  $(precompiled_symbols_hh).tmp $(precompiled_symbols_hh)
	@mv -f $@.tmp $@

$(precompiled_symbols_hh): $(precompiled_symbols_stamp)
	@if test ! -f $@; then					\
	  rm -f $(precompiled_symbols_stamp);			\
	  $(MAKE) $(AM_MAKEFLAGS) $(precompiled_symbols_stamp);	\
	fi
BUILT_SOURCES += $(precompiled_symbols_hh)

## ----------------- ##
## Regular sources.  ##
## ----------------- ##

dist_libuobject@LIBSFX@_la_SOURCES +=		\
  object/barrier.cc				\
  object/centralized-slots.cc			\
  object/code.cc				\
  object/cxx-helper.hh				\
  object/cxx-object.cc				\
  object/date.cc				\
  object/dictionary.cc				\
  object/directory.cc				\
  object/duration.cc				\
  object/event.cc				\
  object/executable.cc				\
  object/file.cc				\
  object/finalizable.cc				\
  object/finalizable.hh				\
  object/float.cc				\
  object/global.cc				\
  object/hash-slots.hh				\
  object/hash-slots.hxx				\
  object/ioservice.cc				\
  object/ioservice.hh				\
  object/job.cc					\
  object/list.cc				\
  object/lobby.cc				\
  object/location.cc				\
  object/object-class.cc			\
  object/object-class.hh			\
  object/object.cc				\
  object/path.cc				\
  object/position.cc				\
  object/primitive.cc				\
  object/root-classes.cc			\
  object/root-classes.hh			\
  object/semaphore.cc				\
  object/semaphore.hh				\
  object/slot.cc				\
  object/socket.cc                              \
  object/socket.hh                              \
  object/server.cc                              \
  object/server.hh                              \
  object/sorted-vector-slots.hh			\
  object/sorted-vector-slots.hxx		\
  object/string.cc				\
  object/symbols.cc				\
  object/symbols.hh				\
  object/system.cc				\
  object/system.hh				\
  object/tag.cc					\
  object/urbi-exception.cc			\
  object/urbi-exception.hh			\
  object/urbi-exception.hxx			\
  object/uvalue.cc				\
  object/uvalue.hh				\
  object/uvar.cc				\
  object/uvar.hh				\
  object/vector-slots.hh			\
  object/vector-slots.hxx

if !COMPILATION_MODE_SPACE
  dist_libuobject@LIBSFX@_la_SOURCES +=		\
    object/format-info.cc			\
    object/format-info.hh			\
    object/format-info.hxx			\
    object/formatter.cc				\
    object/formatter.hh				\
    object/input-stream.cc			\
    object/input-stream.hh			\
    object/output-stream.cc			\
    object/output-stream.hh			\
    object/regexp.cc				\
    object/regexp.hh				\
    object/stream.cc				\
    object/stream.hh

# Too hard currently...
if !WIN32
  dist_libuobject@LIBSFX@_la_SOURCES +=		\
    object/process.cc				\
    object/process.hh
endif !WIN32
endif !COMPILATION_MODE_SPACE
