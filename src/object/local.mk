## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

precompiled_symbols_hh = object/precompiled-symbols.hh
precompiled_symbols_stamp = $(precompiled_symbols_hh:.hh=.stamp)
# We don't include $(nodist_libuobject_la_SOURCES) here, since it
# includes symbols.hh itself.  Currently there seems to be no need to
# support generated files.
precompiled_symbols_hh_deps +=			\
  $(dist_libuobject_la_SOURCES)			\
  object/symbols-generate.pl
EXTRA_DIST += object/symbols-generate.pl
$(precompiled_symbols_stamp): $(precompiled_symbols_hh_deps)
	@rm -f $@.tmp
	@touch $@.tmp
	@echo "rebuilding $(precompiled_symbols_hh) because of:"
	@for i in $?;				\
	do					\
	  echo "       $$i";			\
	done
	:> $(precompiled_symbols_hh)~
	-cp -f $(precompiled_symbols_hh) $(precompiled_symbols_hh)~
	(cd $(top_srcdir) && \
	  perl -w src/object/symbols-generate.pl) >$(precompiled_symbols_hh).tmp
	-diff -u $(precompiled_symbols_hh)~ $(precompiled_symbols_hh).tmp
	$(top_srcdir)/build-aux/move-if-change \
	  $(precompiled_symbols_hh).tmp $(precompiled_symbols_hh)
	@mv -f $@.tmp $@

$(precompiled_symbols_hh): $(precompiled_symbols_stamp)
	@if test ! -f $@; then					\
	  rm -f $(precompiled_symbols_stamp);			\
	  $(MAKE) $(AM_MAKEFLAGS) $(precompiled_symbols_stamp);	\
	fi

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
  object/cxx-primitive.hh			\
  object/cxx-primitive.hxx			\
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
  object/urbi-exception.cc			\
  object/urbi-exception.hh			\
  object/urbi-exception.hxx			\
  object/vector-slots.hh			\
  object/vector-slots.hxx

nodist_libuobject_la_SOURCES +=			\
  object/precompiled-symbols.hh
