#ifndef TESTS_HH
#define TESTS_HH

#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>

/* Liburbi test suite
= Architecture =

Each file corresponds to an 'atomic' test.

Test file layout
- write callbacks, ensure global unique names
- a call to BEGIN_TEST
- code of the test:
  - c++ code using UClient & client, USyncClient &syncClient made available
  - expected output in comments:
   //= <expected output>
- a call to END_TEST
*/



/// display the value
urbi::UCallbackAction dump(const urbi::UMessage & msg);
/// display the value, remove callback if 0
urbi::UCallbackAction removeOnZero(const urbi::UMessage & msg);


#define BEGIN_TEST(name) \
  void name(urbi::UClient & client, urbi::USyncClient & syncClient) {
    
#define END_TEST }

void dispatch(const char * method, urbi::UClient & client, 
	      urbi::USyncClient & syncClient);

#endif
