#include <boost/lexical_cast.hpp>

#include <libport/sysexits.hh>

#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>

#include <tests.hh>

// Use this semaphore in tests that require one.  dump() takes it.
libport::Semaphore dumpSem;

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
  if (getenv("VERBOSE"))
    LIBPORT_ECHO("got a message: " << msg);

  std::cout << char_of(msg.type) << ' ';
  switch (msg.type)
  {
    case urbi::MESSAGE_DATA:
      std::cout << msg.tag << ' ' << *msg.value << std::endl;
      break;

    case urbi::MESSAGE_ERROR:
    case urbi::MESSAGE_SYSTEM:
      std::cout << msg.tag << ' ' << msg.message << std::endl;
      break;
  }
  dumpSem++;
  return urbi::URBI_CONTINUE;
}


urbi::UCallbackAction
removeOnZero(const urbi::UMessage& msg)
{
  if (getenv("VERBOSE"))
    LIBPORT_ECHO("removeOnZero");
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
  if (getenv("VERBOSE"))
    LIBPORT_ECHO("Exiting");
  exit(0);
}


int
main(int argc, const char* argv[])
{
  if (argc < 4)
  {
    std::cerr << "Usage: " << argv[0]
	      << " HOST PORT TEST-NAMES..." << std::endl;
    exit(EX_USAGE);
  }
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
    if (getenv("VERBOSE"))
      LIBPORT_ECHO(argv[0] << ": test " << argv[i]);
    dispatch(argv[i], client, syncClient);
  }
  return 0;
}
