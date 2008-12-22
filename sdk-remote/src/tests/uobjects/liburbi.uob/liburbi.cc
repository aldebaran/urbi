#include <sstream>
#include <urbi/uobject.hh>
#include <urbi/usyncclient.hh>

using namespace urbi;


class liburbi : public UObject
{
public:
  liburbi(const std::string& n)
    : UObject(n)
    , cl(0)
    {
      UBindFunction(liburbi, init);
    }
  ~liburbi()
    {
      delete cl;
    }
  int init()
    {
      UBindFunction(liburbi, connect);
      UBindFunction(liburbi, disconnect);
      UBindFunction(liburbi, connectSame);
      UBindFunction(liburbi, send);
      UBindFunction(liburbi, setCallback);
      return 0;
    }
  int connect(const char* host, int port)
    {
      delete cl;
      cl = new USyncClient(host, port);
      return cl->error();
    }
  int connectSame()
    {
      return connect(getDefaultClient()->getServerName().c_str(),
                     getDefaultClient()->getServerPort());
    }
  int disconnect()
    {
      delete cl;
      cl = 0;
      return 0;
    }
  int send(const std::string& msg)
    {
      cl->send(msg.c_str());
      return cl->error();
    }
  int setCallback(const std::string& tag)
    {
      cl->setCallback(urbi::callback(*this, &liburbi::callback), tag.c_str());
      return 1;
    }
  UCallbackAction callback(const UMessage& msg)
    {
      std::stringstream ss;
      switch(msg.type)
      {
      case MESSAGE_DATA:
        ss << __name << ".onValue(" << *msg.value << "),";
        break;
      case MESSAGE_SYSTEM:
      case MESSAGE_ERROR:
        ss << __name << ".onError(\"" << msg.message << "\"),";
        break;
      }
      send(ss.str());
      return URBI_CONTINUE;
    }
private:
  USyncClient* cl;
};

UStart(liburbi);
