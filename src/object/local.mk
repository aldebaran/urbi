## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

symbols_hh = object/symbols.hh
symbols_stamp = $(symbols_hh:.hh=.stamp)
# We don't include $(nodist_libkernel_la_SOURCES) here, since it
# includes symbols.hh itself.  Currently there seems to be no need to
# support generated files.
symbols_hh_deps += $(dist_libkernel_la_SOURCES)
$(symbols_stamp): $(symbols_hh_deps)
	@rm -f $@.tmp
	@touch $@.tmp
	@echo "rebuilding $(symbols_hh) because of:"
	@for i in $?;				\
	do					\
	  echo "       $$i";			\
	done
	:> $(symbols_hh)~
	-cp -f $(symbols_hh) $(symbols_hh)~
	(cd $(top_srcdir) && perl -w src/object/symbols-generate.pl) >$(symbols_hh).tmp
	-diff -u $(symbols_hh)~ $(symbols_hh).tmp
	$(top_srcdir)/build-aux/move-if-change $(symbols_hh).tmp $(symbols_hh)
	@mv -f $@.tmp $@

$(symbols_hh): $(symbols_stamp)
	@if test ! -f $@; then				\
	  rm -f $(symbols_stamp);			\
	  $(MAKE) $(AM_MAKEFLAGS) $(symbols_stamp);	\
	fi

dist_libkernel_la_SOURCES +=			\
object/alien.hh					\
object/alien.hxx				\
object/alien-class.cc				\
object/alien-class.hh				\
object/atom.hh					\
object/atom.hxx					\
object/code-class.cc				\
object/code-class.hh				\
object/delegate-class.cc                        \
object/delegate-class.hh                        \
object/lobby-class.cc				\
object/lobby-class.hh				\
object/float-class.cc				\
object/float-class.hh				\
object/fwd.hh					\
object/global-class.cc				\
object/global-class.hh				\
object/hash-slots.hh				\
object/hash-slots.hxx				\
object/idelegate.hh				\
object/integer-class.cc				\
object/integer-class.hh				\
object/lazy.hh					\
object/lazy.cc					\
object/list-class.cc				\
object/list-class.hh				\
object/object-class.cc				\
object/object-class.hh				\
object/object.cc				\
object/object.hh				\
object/object.hxx				\
object/primitive-class.cc			\
object/primitive-class.hh			\
object/primitives.cc				\
object/primitives.hh				\
object/scope-class.cc				\
object/scope-class.hh				\
object/sorted-vector-slots.hh			\
object/sorted-vector-slots.hxx			\
object/state.cc					\
object/state.hh					\
object/string-class.cc				\
object/string-class.hh				\
object/symbols.cc				\
object/system-class.cc				\
object/system-class.hh				\
object/tag-class.cc				\
object/tag-class.hh				\
object/task-class.cc				\
object/task-class.hh				\
object/urbi-exception.cc			\
object/urbi-exception.hh			\
object/urbi-exception.hxx

nodist_libkernel_la_SOURCES += 			\
object/symbols.hh
