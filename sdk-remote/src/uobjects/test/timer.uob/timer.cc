/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/debug.hh>
#include <iostream>
#include <urbi/uobject.hh>

GD_CATEGORY(Test.Timer);

urbi::UObjectHub* thub;
class timer: public urbi::UObject
{
public:
  pthread_t mainthread;

  inline void threadCheck()
  {
    aver(mainthread == pthread_self());
  }

  timer(const std::string& name)
    : urbi::UObject(name)
  {
    mainthread = pthread_self();
    UBindVar(timer, updated);
    UBindVar(timer, timerup);
    UBindVar(timer, hupdated);
    hupdated = 0;
    timerup = 0;
    updated = 0;
    UBindFunction(timer, setupUpdate);
    UBindFunction(timer, setupTimer);
    UBindFunction(timer, unsetupTimer);
    UBindFunction(timer, setupHubUpdate);
    UBindFunction(timer, init);
  }
  int init()
  {
    threadCheck();
    return 0;
  }
  int setupUpdate(int d)
  {
    threadCheck();
    USetUpdate(d);
    return 0;
  }
  std::string setupTimer(int d)
  {
    threadCheck();
    return *USetTimer(d, &timer::onTimer);
  }
  bool unsetupTimer(const std::string& s)
  {
    threadCheck();
    return removeTimer(urbi::TimerHandle(new std::string(s)));
  }
  int setupHubUpdate(int d)
  {
    threadCheck();
    thub->USetUpdate(d);
    return 0;
  }
  virtual int update()
  {
    threadCheck();
    updated = (int)updated + 1;
    return 0;
  }
  virtual int onTimer()
  {
    threadCheck();
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
    GD_INFO_DEBUG("timerhub started");
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
