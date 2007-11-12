#include <boost/lexical_cast.hpp>

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
      type =  'D';
      break;

    case urbi::MESSAGE_ERROR:
      type =  'E';
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
  dump(msg);
  if (msg.type == urbi::MESSAGE_DATA
      && msg.value->type == urbi::DATA_DOUBLE
      && msg.value->val == 0)
    return urbi::URBI_REMOVE;
  return urbi::URBI_CONTINUE;
}


int
main(int argc, const char* argv[])
{
  if (argc != 4)
    exit(1);
  const char* host = argv[1];
  int port = boost::lexical_cast<int>(argv[2]);
  const char* call = argv[3];
  urbi::UClient client(host, port);
  urbi::USyncClient syncClient(host, port);
  dispatch(call, client, syncClient);
  return 0;
}
