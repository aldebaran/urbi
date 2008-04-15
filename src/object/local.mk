## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

symbols_hh = $(srcdir)/object/symbols.hh
# We don't include $(nodist_libkernel_la_SOURCES) here, since it
# includes symbols.hh itself.  Currently there seems to be no need to
# support generated files.
symbols.stamp: $(dist_libkernel_la_SOURCES)
	@rm -f $@.tmp
	@touch $@.tmp
	:> $(symbols_hh).old
	-cp -f $(symbols_hh) $(symbols_hh).old
	(cd $(top_srcdir) && perl -w src/object/symbols-generate.pl) >$(symbols_hh).tmp
	-diff -u $(symbols_hh).old $(symbols_hh).tmp
	$(top_srcdir)/build-aux/move-if-change $(symbols_hh).tmp $(symbols_hh)
	@mv -f $@.tmp $@

$(symbols_hh): symbols.stamp
	@if test -f $@; then :; else			\
	  rm -f symbols.stamp; 				\
	  $(MAKE) $(AM_MAKEFLAGS) symbols.stamp;	\
	fi

dist_libkernel_la_SOURCES +=			\
object/alien.hh					\
object/alien.hxx				\
object/alien-class.cc				\
object/alien-class.hh				\
object/atom.hh					\
object/atom.hxx					\
object/call-class.cc				\
object/call-class.hh				\
object/code-class.cc				\
object/code-class.hh				\
object/delegate-class.cc                        \
object/delegate-class.hh                        \
object/lobby-class.cc				\
object/lobby-class.hh				\
object/float-class.cc				\
object/float-class.hh				\
object/global-class.cc				\
object/global-class.hh				\
object/fwd.hh					\
object/idelegate.hh				\
object/integer-class.cc				\
object/integer-class.hh				\
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
object/state.cc					\
object/state.hh					\
object/scope-class.cc				\
object/scope-class.hh				\
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
