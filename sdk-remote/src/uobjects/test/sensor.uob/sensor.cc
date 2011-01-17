/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <iostream>

#include "sensor.hh"

UStart(sensor);

sensor::sensor(std::string s)
  : UObject(s)
{
  UBindVar(sensor, val);
  USensor(val);
  val = 4;
  UBindFunction(sensor, init);
  UBindFunction(sensor, setVal);
  UNotifyChange(val, &sensor::newval);
  UNotifyAccess(val, &sensor::getval);
}

int
sensor::init()
{
  return 0;
}

UReturn
sensor::newval(UVar& v)
{
  std::cout << (int)v << std::endl;
  return 0;
}

void
sensor::setVal(int v)
{
  val = v;
}

UReturn
sensor::getval(UVar& v)
{
  v = 666;
  std::cout << "set value to 666" << std::endl;
  return 0;
}
