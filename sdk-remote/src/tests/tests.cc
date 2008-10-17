#include <boost/lexical_cast.hpp>

#include <libport/sysexits.hh>

#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>

#include <tests.hh>

// Use this semaphore in tests that require one.  dump() takes it.
libport::Semaphore dumpSem;

// argv[0].
const char* program_name = "unknown program";

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
  if (msg.tag == "start" || msg.tag == "ident")
    return urbi::URBI_CONTINUE;

  VERBOSE("got a message: " << msg);
  std::cout << char_of(msg.type)
	    << ' ' << msg.tag
	    << ' ';
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


int
main(int argc, const char* argv[])
{
  // Actually argv[0] is verbose and not interesting.
  program_name = "tests";
  if (argc < 4)
    std::cerr << "Usage: " << program_name
	      << " HOST PORT TEST-NAMES..." << std::endl
	      << libport::exit(EX_USAGE);

  const char* host = argv[1];
  int port = boost::lexical_cast<int>(argv[2]);

  urbi::UClient client(host, port);
  if (client.error())
    std::cerr << "Failed to set up properly the client" << std::endl
              << libport::exit(EX_SOFTWARE);

  urbi::USyncClient syncClient(host, port);
  client.setClientErrorCallback(callback(&doExit));
  if (syncClient.error())
    std::cerr << "Failed to set up properly the syncClient" << std::endl
              << libport::exit(EX_SOFTWARE);

  for (int i = 3; i < argc; ++i)
  {
    VERBOSE("test " << argv[i]);
    dispatch(argv[i], client, syncClient);
  }
}
