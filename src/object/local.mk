## ---------------------- ##
## List of used symbols.  ##
## ---------------------- ##

nodist_libuobject_la_SOURCES += 		\
  $(precompiled_symbols_hh)

# We generated this file in builddir so that we can have a single
# srcdir, but several builddir with different configuration-options
# that may result in a different set of precompiled symbols.
precompiled_symbols_hh = object/precompiled-symbols.hh
precompiled_symbols_stamp = $(precompiled_symbols_hh:.hh=.stamp)
# filter-out generated files, and precompiled_symbols_hh itself to
# avoid circular dependencies.
precompiled_symbols_hh_sources =		\
  parser/utoken.l				\
  parser/ugrammar.y				\
  $(filter-out $(precompiled_symbols_hh)	\
               $(FROM_UGRAMMAR_Y)		\
               $(FROM_UTOKEN_L),		\
        $(dist_libuobject_la_SOURCES))
EXTRA_DIST += object/symbols-generate.pl $(precompiled_symbols_stamp)

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
	if test -f $(precompiled_symbols_hh); then	\
	  cat $(precompiled_symbols_hh);		\
	fi >$(precompiled_symbols_hh)~
	(cd $(srcdir) &&				\
	 ./object/symbols-generate.pl			\
		$(precompiled_symbols_hh_sources))	\
		>$(precompiled_symbols_hh).tmp
	diff -u $(precompiled_symbols_hh)~ $(precompiled_symbols_hh).tmp || true
	$(move_if_change)						\
	  $(precompiled_symbols_hh).tmp $(precompiled_symbols_hh)
	@mv -f $@.tmp $@

$(precompiled_symbols_hh): $(precompiled_symbols_stamp)
	@if test ! -f $@; then					\
	  rm -f $(precompiled_symbols_stamp);			\
	  $(MAKE) $(AM_MAKEFLAGS) $(precompiled_symbols_stamp);	\
	fi
BUILT_SOURCES += $(precompiled_symbols_hh)

## ------------------------ ##
## Generated source files.  ##
## ------------------------ ##

FROM_PY =					\
  object/any-to-boost-function.hxx		\
  object/cxx-primitive.hxx			\
  object/executable.hh

nodist_libuobject_la_SOURCES += $(FROM_PY)
EXTRA_DIST += $(FROM_PY:=.py)

%.hxx: %.hxx.py
	rm -f $@ $@.tmp
	$< > $@.tmp
	chmod a-w $@.tmp
	mv $@.tmp $@

%.hh: %.hh.py
	rm -f $@ $@.tmp
	$< > $@.tmp
	chmod a-w $@.tmp
	mv $@.tmp $@

## ----------------- ##
## Regular sources.  ##
## ----------------- ##

dist_libuobject_la_SOURCES +=			\
  object/any-to-boost-function.hh		\
  object/barrier.cc				\
  object/barrier.hh				\
  object/centralized-slots.hh			\
  object/centralized-slots.hxx			\
  object/centralized-slots.cc			\
  object/code.cc				\
  object/code.hh				\
  object/cxx-conversions.hh			\
  object/cxx-conversions.hxx			\
  object/cxx-helper.hh				\
  object/cxx-object.cc				\
  object/cxx-object.hh				\
  object/cxx-object.hxx				\
  object/cxx-primitive.hh			\
  object/dictionary.cc				\
  object/dictionary.hh				\
  object/directory.cc				\
  object/directory.hh				\
  object/executable.cc				\
  object/file.cc				\
  object/file.hh				\
  object/finalizable.hh                         \
  object/finalizable.cc                         \
  object/float.cc				\
  object/float.hh				\
  object/format-info.cc				\
  object/format-info.hh				\
  object/format-info.hxx			\
  object/formatter.cc				\
  object/formatter.hh				\
  object/fwd.hh					\
  object/global.cc				\
  object/global.hh				\
  object/hash-slots.hh				\
  object/hash-slots.hxx				\
  object/list.cc				\
  object/list.hh				\
  object/lobby.cc				\
  object/lobby.hh				\
  object/object-class.cc			\
  object/object-class.hh			\
  object/object.cc				\
  object/object.hh				\
  object/object.hxx				\
  object/path.cc				\
  object/path.hh				\
  object/primitive.cc				\
  object/primitive.hh				\
  object/root-classes.cc			\
  object/root-classes.hh			\
  object/semaphore.cc				\
  object/semaphore.hh				\
  object/slot.cc				\
  object/slot.hh				\
  object/slot.hxx				\
  object/sorted-vector-slots.hh			\
  object/sorted-vector-slots.hxx		\
  object/string.cc				\
  object/string.hh				\
  object/symbols.cc				\
  object/symbols.hh				\
  object/system.cc				\
  object/system.hh				\
  object/tag.cc					\
  object/tag.hh					\
  object/task.cc				\
  object/task.hh				\
  object/urbi-exception.hh			\
  object/urbi-exception.hxx			\
  object/uvar.hh                                \
  object/uvar.cc                                \
  object/vector-slots.hh			\
  object/vector-slots.hxx

if !COMPILATION_MODE_SPACE
  dist_libuobject_la_SOURCES +=			\
    object/output-stream.cc			\
    object/output-stream.hh			\
    object/socket.cc				\
    object/socket.hh				\
    object/server.cc				\
    object/server.hh
endif
