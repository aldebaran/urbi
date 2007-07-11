## Process this file with automake --
include $(top_srcdir)/src/kernel.mk

noinst_LTLIBRARIES = libobject.la

libobject_la_SOURCES =				\
atom.hh						\
atom.hxx					\
atom.cc						\
object.hh					\
object.hxx					\
object.cc

check_HEADERS = object-test.hh
check_PROGRAMS =				\
		object-test			\
		object-test-parent		\
		object-test-lookup		\
		object-test-slot

object_test_SOURCES = object-test.cc
object_test_parent_SOURCES = object-test-parent.cc
object_test_lookup_SOURCES = object-test-lookup.cc
object_test_slot_SOURCES = object-test-slot.cc

LDADD = libobject.la $(libport_la)
TESTS = $(check_PROGRAMS)
