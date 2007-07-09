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

check_PROGRAMS = object-test
object_test_SOURCES = object-test.cc
LDADD = libobject.la $(libport_la)
TESTS = $(check_PROGRAMS)
