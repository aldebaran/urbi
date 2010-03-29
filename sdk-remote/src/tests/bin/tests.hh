/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef SDK_REMOTE_TESTS_TESTS_HH
# define SDK_REMOTE_TESTS_TESTS_HH

# include <iostream>

# include <libport/debug.hh>
# include <libport/semaphore.hh>
# include <libport/program-name.hh>
# include <libport/unistd.h>

# include <urbi/uclient.hh>
# include <urbi/usyncclient.hh>

using libport::program_name;

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

/// Register the message category for each file including this header.
GD_ADD_CATEGORY(TEST);

/// Display a debug message.
#define VERBOSE(S)                              \
  do {                                          \
    GD_CATEGORY(TEST);                          \
    GD_SINFO(program_name () << ": " << S);     \
  } while (0)

/// Send S to the Client.
#define SEND_(Client, S)                                \
  do {							\
    if (Client.isConnected())                           \
    {                                                   \
      VERBOSE("Send: " << S);                           \
      Client.send("%s\n", (S));                         \
    }                                                   \
    else                                                \
      VERBOSE("Not connected, cannot send: " << S);     \
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
  urbi::getValue(c.syncGet(msg), res);
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

#define BEGIN_TEST                              \
  void                                          \
  test(urbi::UClient& client,                   \
       urbi::USyncClient& syncClient)           \
  {

#define END_TEST                                \
  }

void test(urbi::UClient& client, urbi::USyncClient& syncClient);

#endif // SDK_REMOTE_TESTS_TESTS_HH
