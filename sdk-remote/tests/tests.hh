#ifndef TESTS_HH
#define TESTS_HH
#include <libport/semaphore.hh>
#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>

/* Liburbi test suite
= Architecture =

Each file corresponds to an 'atomic' test.

Test file layout
- #includes "tests.hh"
- write callbacks, ensure global unique names
- a call to BEGIN_TEST(testname),  testname must be filename-without-extension
- code of the test:
  - c++ code using UClient & client, USyncClient &syncClient made available
  - expected output in comments:
   //= <expected output>
- a call to END_TEST
*/


extern libport::Semaphore dumpSem;
/// display the value, increment dumpSem.
urbi::UCallbackAction dump(const urbi::UMessage & msg);
/// display the value, incremente dumpSem remove callback if 0
urbi::UCallbackAction removeOnZero(const urbi::UMessage & msg);


#define BEGIN_TEST(name) \
  void name(urbi::UClient & client, urbi::USyncClient & syncClient) {
    
#define END_TEST \
  client.send("shutdown;"); \
  }

void dispatch(const char * method, urbi::UClient & client, 
	      urbi::USyncClient & syncClient);

#endif
