#include <boost/lexical_cast.hpp>

#include <libport/sysexits.hh>

#include <urbi/uclient.hh>
#include <urbi/usyncclient.hh>

#include "tests.hh"

libport::Semaphore dumpSem;

urbi::UCallbackAction
dump(const urbi::UMessage& msg)
{
  unsigned char type;
  switch (msg.type)
  {
    case urbi::MESSAGE_DATA:
      type = 'D';
      break;
    case urbi::MESSAGE_ERROR:
      type = 'E';
      break;
    case urbi::MESSAGE_SYSTEM:
      type = 'S';
      break;
    default:
      type = '?';
  }

  std::cout << type << ' ';
  switch (msg.type)
  {
    case urbi::MESSAGE_DATA:
      std::cout << msg.tag << ' ' << *msg.value << std::endl;
      break;

    case urbi::MESSAGE_ERROR:
    case urbi::MESSAGE_SYSTEM:
      std::cout  << msg.tag << ' ' << msg.message << std::endl;
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
  urbi::USyncClient syncClient(host, port);
  client.setClientErrorCallback(callback(&doExit));
  for (int i = 3; i < argc; ++i)
  {
    if (getenv("VERBOSE"))
      LIBPORT_ECHO(argv[0] << ": test " << argv[i]);
    dispatch(argv[i], client, syncClient);
  }
  return 0;
}
