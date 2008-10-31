## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

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
	:> $(precompiled_symbols_hh)~
	-cp -f $(srcdir)/$(precompiled_symbols_hh) $(precompiled_symbols_hh)~
	(cd $(srcdir) &&				\
	 ./object/symbols-generate.pl			\
		$(precompiled_symbols_hh_sources))	\
		>$(precompiled_symbols_hh).tmp
	diff -u $(precompiled_symbols_hh)~ $(precompiled_symbols_hh).tmp || true
	$(top_srcdir)/build-aux/move-if-change \
	  $(precompiled_symbols_hh).tmp $(srcdir)/$(precompiled_symbols_hh)
	@mv -f $@.tmp $(srcdir)/$@

$(precompiled_symbols_hh): $(precompiled_symbols_stamp)
	@if test ! -f $@; then					\
	  rm -f $(precompiled_symbols_stamp);			\
	  $(MAKE) $(AM_MAKEFLAGS) $(precompiled_symbols_stamp);	\
	fi

%.hxx: %.hxx.py
	rm -f $@ $@.tmp
	$< > $@.tmp
	chmod a-w $@.tmp
	mv $@.tmp $@

# cxx_primitives_hxx = object/cxx-primitive.hxx
# cxx_primitives_hxx_py = $(srcdir)/$(cxx_primitives_hxx).py
# $(cxx_primitives_hxx): $(cxx_primitives_hxx_py)
# 	$(cxx_primitives_hxx_py) > $(cxx_primitives_hxx).tmp
# 	mv -f $(cxx_primitives_hxx).tmp $(cxx_primitives_hxx)
# 	chmod a-w $(cxx_primitives_hxx)

# any_to_boost_hxx = object/any-to-boost-function.hxx
# any_to_boost_hxx_py = $(srcdir)/$(any_to_boost_hxx).py
# $(any_to_boost_hxx): $(any_to_boost_hxx_py)
# 	$(any_to_boost_hxx_py) > $(any_to_boost_hxx).tmp
# 	mv -f $(any_to_boost_hxx).tmp $(any_to_boost_hxx)
# 	chmod a-w $(any_to_boost_hxx)


BUILT_SOURCES +=				\
  $(precompiled_symbols_hh)			\
  $(cxx_primitives_hxx)				\
  $(any_to_boost_hxx)

dist_libuobject_la_SOURCES +=			\
  object/any-to-boost-function.hh		\
  object/any-to-boost-function.hxx		\
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
  object/file.cc				\
  object/file.hh				\
  object/float.cc				\
  object/float.hh				\
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
  object/primitives.hh				\
  object/root-classes.cc			\
  object/root-classes.hh			\
  object/semaphore.cc				\
  object/semaphore.hh				\
  object/slots.hh				\
  object/sorted-vector-slots.hh			\
  object/sorted-vector-slots.hxx		\
  object/state.cc				\
  object/state.hh				\
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
  object/vector-slots.hh			\
  object/vector-slots.hxx			\
  $(precompiled_symbols_hh)
