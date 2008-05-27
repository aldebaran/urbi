#ifndef TESTS_HH
#define TESTS_HH
#include <libport/semaphore.hh>
#include <libport/compiler.hh>
#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>

/* Liburbi test suite
= Architecture =

Each file corresponds to an 'atomic' test.

Test file layout
- #includes "tests.hh"
- write callbacks, ensure global unique names
- a call to BEGIN_TEST(testname, clientname, syncclientname),  
   testname must be filename-without-extension
- code of the test:
- setup callbacks, the callback function dump is provided: it displays the
  message and increments dumpSem semaphore. Setting a Wildcard or at list
  an error callback is recomended.
  - c++ code using UClient & client, USyncClient &syncClient made available
  - expected output in comments:
   //= <expected output>
- a call to END_TEST

<expected output>
  <type> <tag> <value>
<type>
  D|E|S for data, error, system
*/


extern libport::Semaphore dumpSem;
/// display the value, increment dumpSem.
urbi::UCallbackAction dump(const urbi::UMessage & msg);
/// display the value, incremente dumpSem remove callback if 0
urbi::UCallbackAction removeOnZero(const urbi::UMessage & msg);


#define BEGIN_TEST(name, clientName, syncClientName)		\
  void name(urbi::UClient & clientName, urbi::USyncClient & syncClientName) {\
    if (getenv("VERBOSE")) LIBPORT_ECHO("starting test " << #name );

#define END_TEST \
  if (getenv("VERBOSE")) LIBPORT_ECHO("sending 'shutdown'");  \
  client.send("shutdown;"); \
  }

void dispatch(const char * method, urbi::UClient & client,
	      urbi::USyncClient & syncClient);

/// \a Name is the base name of the C++ file containing the function
/// \a Name.
#define TESTS_RUN(Name)							\
  do {									\
    if (!strcmp(method, #Name))						\
    {									\
      void Name (urbi::UClient& , urbi::USyncClient&);			\
      Name(client, syncClient);						\
      return;								\
    }									\
  } while (0)

#endif
