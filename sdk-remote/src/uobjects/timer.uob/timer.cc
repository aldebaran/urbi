/*
 * Copyright (C) 2009, Gostai S.A.S.
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

GD_ADD_CATEGORY(timer);

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
    GD_CATEGORY(timer);
    GD_ERROR("timerhub started");
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
