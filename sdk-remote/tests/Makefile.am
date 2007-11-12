include $(top_srcdir)/flags.mk
include $(top_srcdir)/build-aux/check.mk

# From a test file to a log file.
# Do not use a regular `.test.log:' rule here, since in that case the
# following rule (without incoming extension) will mask this one.
%.log: %.cc
	@$(am__check_pre) $(srcdir)/liburbi-check $(srcdir)/$* $(am__check_post)

AM_CPPFLAGS += $(BOOST_CPPFLAGS)

AM_LDADD = $(top_builddir)/src/liburbi/liburbi.la 	\
           $(top_builddir)/jpeg-6b/libjpeg.la		\
           $(PTHREAD_LIBS)

# This is a hack for Automake 1.9: 1.10 honors AM_LDADD.
LDADD = $(AM_LDADD)

check_PROGRAMS = tests
INSTRUMENT = true
TESTS_ENVIRONMENT =				\
 srcdir='$(srcdir)'				\
 INSTRUMENT=$(INSTRUMENT)			\
 PACKAGE_NAME='$(PACKAGE_TARNAME)'		\
 URBI_PATH='$(top_srcdir)/share/urbi'

TESTS = checkfiles/values.cc \
	checkfiles/removecallbacks.cc
TEST_LOGS = $(TESTS:.cc=.log)
tests_SOURCES = tests.hh tests.cc $(TESTS)
tests_CPPFLAGS = $(AM_CPPFLAGS) -I$(srcdir)
tests_CXXFLAGS = $(AM_CXXFLAGS) -Wno-unused-parameter
nodist_tests_SOURCES = dispatcher.cc

EXTRA_DIST += liburbi-check
CLEANFILES += $(nodist_tests_SOURCES)
dispatcher.cc: $(tests_SOURCES) Makefile
	(                                                                    \
      	echo '#include "tests.hh"'  ; \
	for f in $(TESTS); do  \
	  name=$$(echo $$f | sed -e 's@.*/@@' -e 's@\..*@@') ;                \
	  echo "void $$name(urbi::UClient & , urbi::USyncClient &);";\
	done;    \
  	echo 'void dispatch(const char * method, urbi::UClient & client,'   ;\
	echo '    urbi::USyncClient & syncClient) {'   ;\
	for f in $(TESTS); do  \
	  name=$$(echo $$f | sed -e 's@.*/@@' -e 's@\..*@@') ;                \
	  echo 'if (!strcmp(method, '"\"$$name\")) { \
	    $$name(client, syncClient);return;}" ;\
	done; echo 'std::cerr << "unknown test " <<method << std::endl;}' ;   \
	) > dispatcher.cc

.PHONY: clean-test-dirs
clean-local: clean-test-dirs
clean-test-dirs:
	rm -rf $(TESTS:.cc=.dir)

