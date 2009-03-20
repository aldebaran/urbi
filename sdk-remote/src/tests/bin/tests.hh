#ifndef SDK_REMOTE_TESTS_TESTS_HH
# define SDK_REMOTE_TESTS_TESTS_HH

# include <libport/semaphore.hh>
# include <libport/program-name.hh>
# include <libport/unistd.h>
# include <urbi/uclient.hh>
# include <urbi/usyncclient.hh>

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


/// Display a debug message.
#define VERBOSE(S)				\
  std::cerr << libport::program_name() << ": " << S << std::endl

/// Send S to the client.
#define SEND(S)						\
  do {							\
    VERBOSE("Sending: " << S);				\
    client.send((std::string(S) + "\n").c_str());	\
  } while (0)


extern libport::Semaphore dumpSem;

/// display the value, increment dumpSem.
urbi::UCallbackAction dump(const urbi::UMessage& msg);
/// display the value, incremente dumpSem remove callback if 0
urbi::UCallbackAction removeOnZero(const urbi::UMessage& msg);


#define BEGIN_TEST(Name, ClientName, SyncClientName)    \
  void                                                  \
  Name(urbi::UClient& ClientName,                       \
       urbi::USyncClient& SyncClientName)               \
  {                                                     \
    VERBOSE("starting test " << #Name);

#define END_TEST                                \
    sleep(3);                                   \
    SEND("shutdown;");                          \
  }

void dispatch(const std::string& method, urbi::UClient& client,
	      urbi::USyncClient& syncClient);

/// \a Name is the base name of the C++ file containing the function
/// \a Name.
#define TESTS_RUN(Name)							\
  do {									\
    if (method == #Name)						\
    {									\
      void Name(urbi::UClient& , urbi::USyncClient&);			\
      Name(client, syncClient);						\
      return;								\
    }									\
  } while (0)

#endif // SDK_REMOTE_TESTS_TESTS_HH
