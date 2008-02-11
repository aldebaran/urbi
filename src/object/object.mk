## ------------------------------------- ##
## Generate the list of symbols we use.  ##
## ------------------------------------- ##

.PHONY: symbols
symbols:
	mv -f $(srcdir)/object/symbols.hh $(srcdir)/object/symbols.hh.old
	$(MAKE) $(AM_MAKEFLAGS) $(srcdir)/object/symbols.hh
$(srcdir)/object/symbols.hh:
	(cd $(top_srcdir) && perl -w src/object/symbols-generate.pl) >$@.tmp
	-diff -u $@.tmp $@.old
	$(top_srcdir)/build-aux/move-if-change $@.tmp $@


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
object/string-class.cc				\
object/string-class.hh				\
object/symbols.cc				\
object/symbols.hh				\
object/task-class.cc				\
object/task-class.hh				\
object/urbi-exception.cc			\
object/urbi-exception.hh			\
object/urbi-exception.hxx
