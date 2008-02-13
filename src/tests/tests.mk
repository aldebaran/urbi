noinst_PROGRAMS += twoCoroTest

twoCoroTest_SOURCES = tests/twoCoroTest.c
twoCoroTest_CFLAGS = -I$(srcdir)/scheduler/libcoroutine
twoCoroTest_LDADD = libkernel.la
