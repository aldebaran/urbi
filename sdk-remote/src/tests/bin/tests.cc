/*
 * Copyright (C) 2007-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/lexical-cast.hh>

#include <libport/cli.hh>
#include <libport/debug.hh>
#include <libport/foreach.hh>
#include <libport/sysexits.hh>

#include <urbi/uabstractclient.hh>
#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>

#include <bin/tests.hh>

GD_INIT();

// Use this semaphore in tests that require one.  dump() takes it.
libport::Semaphore dumpSem;

static
void
usage()
{
  std::cout <<
    "Usage: " << program_name() << " [OPTION]... TEST...\n"
    "\n"
    "Options:\n"
    "  -h, --help            output this message and exit successfully\n"
    "  -H, --host ADDR   the host running the Urbi server"
              << " [" << urbi::UClient::default_host() << "]\n"
    "  -p, --port PORT       tcp port URBI will listen to"
              << " [" << urbi::UClient::URBI_PORT << "]\n"
    "  -r, --port-file FILE  file containing the port to listen to\n"
              << libport::exit(EX_OK);
}

static char
char_of(urbi::UMessageType t)
{
  switch (t)
  {
    case urbi::MESSAGE_DATA:   return 'D';
    case urbi::MESSAGE_ERROR:  return 'E';
    case urbi::MESSAGE_SYSTEM: return 'S';
    default:                   return '?';
  }
}

urbi::UCallbackAction
log(const urbi::UMessage& msg)
{
  GD_SINFO("Recv: " << msg);
  return urbi::URBI_CONTINUE;
}

urbi::UCallbackAction
dump(const urbi::UMessage& msg)
{
  log(msg);

  std::cout << char_of(msg.type) << ' ' << msg.tag << ' ';
  switch (msg.type)
  {
    case urbi::MESSAGE_DATA:
      std::cout << *msg.value << std::endl;
      break;

    case urbi::MESSAGE_ERROR:
    case urbi::MESSAGE_SYSTEM:
      std::cout << msg.message << std::endl;
      break;
  }

  dumpSem++;
  return urbi::URBI_CONTINUE;
}


urbi::UCallbackAction
removeOnZero(const urbi::UMessage& msg)
{
  GD_INFO("removeOnZero");
  dump(msg);
  if (msg.type == urbi::MESSAGE_DATA
      && msg.value->type == urbi::DATA_DOUBLE
      && msg.value->val == 0)
    return urbi::URBI_REMOVE;
  return urbi::URBI_CONTINUE;
}

std::string
sget_error(urbi::USyncClient& c, const std::string& msg)
{
  GD_SINFO("sget_error: Asking " << msg);
  urbi::UMessage* m = c.syncGet(msg);
  aver(m && m->type == urbi::MESSAGE_ERROR);
  std::string res(m->message);
  delete m;
  return res;
}


// Cannot make a template here, as we would like to return by copy.
# define MAKE_CLIENT(Type, Name)                                        \
  urbi::Type Name(host, port);                                          \
  GD_FINFO(#Name "(%s, %s) @ %s", host, port, &Name);                   \
  Name.setErrorCallback(urbi::callback(&log));                          \
  Name.setCallback(urbi::callback(&log), "log");                        \
  if (Name.error())                                                     \
    std::cerr << "Failed to set up properly the " #Name << std::endl    \
              << libport::exit(EX_SOFTWARE);                            \
  /* Wait to be set up. */                                              \
  Name.waitForKernelVersion();                                          \
  GD_INFO(#Name " received Kernel Version")


int
main(int argc, char* argv[])
{
  // argv[0] = basename(argv[0]).
  const char* argv0 = argv[0];
  if (char* cp = strrchr(argv[0], '/'))
    argv[0] = cp + 1;
  libport::program_initialize(argc, argv);
  std::string host = urbi::UClient::default_host();
  int port = urbi::UAbstractClient::URBI_PORT;

  GD_SINFO("This is " << argv0);
  GD_INFO("Processing option");
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
      host = libport::convert_argument<std::string>(arg, argv[++i]);
    else if (arg == "--port" || arg == "-P")
      port = libport::convert_argument<unsigned>(arg, argv[++i]);
    else if (arg == "--port-file" || arg == "-r")
      port =
        (libport::file_contents_get<int>
         (libport::convert_argument<const char*>(arg, argv[++i])));
    else
      libport::invalid_option(arg);
  }

  MAKE_CLIENT(UClient, client);
  MAKE_CLIENT(USyncClient, syncClient);

  GD_INFO("Starting");
  test(client, syncClient);

  GD_INFO("Epilogue, Sleep 3s");
  sleep(3);

  GD_INFO("Shutting down");
  // Handle the case when the other connection is down.
  SSEND("disown({ sleep(0.5); shutdown })|; quit;");
  SEND("shutdown;");

  // Don't close the connection too soon, as it may result in the
  // "shutdown" messages to be dropped when the connection is cut.
  // FIXME: rather, wait for the deconnection from the server.
  sleep(1);
  GD_INFO("End");
}
