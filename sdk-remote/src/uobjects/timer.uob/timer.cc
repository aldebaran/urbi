#include <iostream>
#include <urbi/uobject.hh>

urbi::UObjectHub* thub;
class timer: public urbi::UObject
{
public:
  timer(const std::string& name)
    : urbi::UObject(name)
    {
      UBindVar(timer, updated);
      UBindVar(timer, timerup);
      UBindVar(timer, hupdated);
      hupdated = 0;
      timerup = 0;
      updated = 0;
      UBindFunction(timer, setupUpdate);
      UBindFunction(timer, setupTimer);
      UBindFunction(timer, setupHubUpdate);
      UBindFunction(timer, init);
    }
    int init()
    {
      return 0;
    }
    int setupUpdate(int d)
    {
      USetUpdate(d);
      return 0;
    }
    int setupTimer(int d)
    {
      USetTimer(d, &timer::onTimer);
      return 0;
    }
    int setupHubUpdate(int d)
    {
      thub->USetUpdate(d);
      return 0;
    }
    virtual int update()
    {
      updated = (int)updated + 1;
      return 0;
    }
    virtual int onTimer()
    {
      timerup = (int)timerup + 1;
      return 0;
    }
    urbi::UVar updated;
    urbi::UVar hupdated;
    urbi::UVar timerup;
};

class timerHub: public urbi::UObjectHub
{
  public:
    timerHub(const std::string& name)
	: urbi::UObjectHub(name)
	{
	  std::cerr <<"timerhub started"<<std::endl;
	  thub = this;
	}
	virtual int update()
	{
	  urbi::UVar v("timer","hupdated");
	  v = 1;
	  return 0;
	}
};
UStart(timer);
UStartHub(timerHub);
