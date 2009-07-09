#ifndef SDK_REMOTE_TESTS_TESTS_HH
# define SDK_REMOTE_TESTS_TESTS_HH

# include <libport/debug.hh>
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
  message and increments dumpSem semaphore. Setting a Wildcard or at least
  an error callback is recommended.
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
#define VERBOSE(S)                              \
  do {                                          \
    GD_CATEGORY(TEST);                          \
    std::ostringstream o;                       \
    o << S;                                     \
    GD_INFO(o.str());                           \
  } while (0)

/// Send S to the Client.
#define SEND_(Client, S)                                \
  do {							\
    VERBOSE("Sending: " << S);				\
    Client.send((std::string(S) + "\n").c_str());	\
  } while (0)

/// Send S to client/syncclient.
#define SEND(S)	  SEND_(client, S)
#define SSEND(S)  SEND_(syncClient, S)


/*----------.
| SyncGet.  |
`----------*/

template <typename T>
inline
T
sget(urbi::USyncClient& c, const std::string& msg)
{
  VERBOSE("syncGet: Asking " << msg);
  T res;
  assert(urbi::getValue(c.syncGet(msg), res));
  return res;
}

/// syncGet E from the syncClient.
#define SGET(Type, E)                           \
  sget<Type>(syncClient, E)


std::string sget_error(urbi::USyncClient& c, const std::string& msg);

/// Send a computation, expect an error.
#define SGET_ERROR(E)                           \
  sget_error(syncClient, E)



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
    SSEND("quit;");                             \
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
      void Name(urbi::UClient&, urbi::USyncClient&);			\
      Name(client, syncClient);						\
      return;								\
    }									\
  } while (0)

#endif // SDK_REMOTE_TESTS_TESTS_HH
