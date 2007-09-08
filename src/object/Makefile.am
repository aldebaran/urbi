## Process this file with automake --
include $(top_srcdir)/src/kernel.mk

AM_LDFLAGS += -no-undefined

noinst_LTLIBRARIES = libobject.la

libobject_la_SOURCES =				\
atom.hh						\
atom.hxx					\
object.hh					\
object.hxx					\
object.cc					\
primitives.hh					\
primitives.cc					\
code-class.hh					\
code-class.cc					\
context-class.hh				\
context-class.cc				\
float-class.hh					\
float-class.cc					\
integer-class.hh				\
integer-class.cc				\
list-class.hh					\
list-class.cc					\
object-class.hh					\
object-class.cc					\
primitive-class.hh				\
primitive-class.cc				\
state.hh					\
state.cc					\
string-class.hh					\
string-class.cc					\
urbi-exception.hh				\
urbi-exception.cc



## ------- ##
## Tests.  ##
## ------- ##

check_HEADERS = object-test.hh
check_PROGRAMS =				\
object-test					\
object-test-parent				\
object-test-lookup				\
object-test-slot				\
object-test-primitives

object_test_SOURCES = object-test.cc
object_test_lookup_SOURCES = object-test-lookup.cc
object_test_parent_SOURCES = object-test-parent.cc
object_test_primitives_SOURCES = object-test-primitives.cc
object_test_slot_SOURCES = object-test-slot.cc

LDADD = libobject.la ../ast/libast.la ../libkernel.la $(libport_la)
TESTS = $(check_PROGRAMS)
