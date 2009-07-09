#include <boost/lexical_cast.hpp>

#include <libport/cli.hh>
#include <libport/debug.hh>
#include <libport/foreach.hh>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>

#include <urbi/uabstractclient.hh>
#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>

#include <bin/tests.hh>

using libport::program_name;

GD_INIT();
GD_ADD_CATEGORY(TEST);

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
    "  -h, --help            display this message and exit\n"
    "  -H, --host ADDR   the host running the Urbi server"
              << " [" << urbi::UClient::DEFAULT_HOST << "]\n"
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
dump(const urbi::UMessage& msg)
{
  VERBOSE("got a message: " << msg);
  if (msg.tag == "start" || msg.tag == "ident")
    return urbi::URBI_CONTINUE;

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
  VERBOSE("removeOnZero");
  dump(msg);
  if (msg.type == urbi::MESSAGE_DATA
      && msg.value->type == urbi::DATA_DOUBLE
      && msg.value->val == 0)
    return urbi::URBI_REMOVE;
  return urbi::URBI_CONTINUE;
}

urbi::UCallbackAction
doExit(const urbi::UMessage& /* msg */)
{
  VERBOSE("Exiting");
  exit(0);
}


std::string
sget_error(urbi::USyncClient& c, const std::string& msg)
{
  VERBOSE("get_erroneous: Asking " << msg);
  urbi::UMessage* m = c.syncGet(msg);
  assert(m && m->type == urbi::MESSAGE_ERROR);
  std::string res(m->message);
  delete m;
  return res;
}


int
main(int argc, char* argv[])
{
  GD_CATEGORY(Test);

  // Actually argv[0] is verbose and not interesting.
  libport::program_initialize(argc, argv);
  std::string host = urbi::UClient::DEFAULT_HOST;
  int port = urbi::UAbstractClient::URBI_PORT;

  /// The command line test requested.
  std::vector<std::string> tests;

  VERBOSE("Processing option");
  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg == "--help" || arg == "-h")
      usage();
    else if (arg == "--host" || arg == "-H")
      host = libport::convert_argument<std::string>(arg, argv[++i]);
    else if (arg == "--port" || arg == "-p")
      port = libport::convert_argument<unsigned>(arg, argv[++i]);
    else if (arg == "--port-file" || arg == "-r")
      port =
        (libport::file_contents_get<int>
         (libport::convert_argument<const char*>(arg, argv[++i])));
    else if (arg[0] == '-')
      libport::invalid_option(arg);
    else
      tests.push_back(arg);
  }

  urbi::UClient client(host, port);
  VERBOSE("client(" << host << ", " << port << ") @ " << &client);
  client.setClientErrorCallback(callback(&doExit));
  if (client.error())
    std::cerr << "Failed to set up properly the client" << std::endl
              << libport::exit(EX_SOFTWARE);

  urbi::USyncClient syncClient(host, port);
  VERBOSE("syncClient(" << host << ", " << port << ") @ " << &syncClient);
  if (syncClient.error())
    std::cerr << "Failed to set up properly the syncClient" << std::endl
              << libport::exit(EX_SOFTWARE);

  foreach (const std::string& s, tests)
  {
    VERBOSE("test " << s);
    dispatch(s, client, syncClient);
  }
}
