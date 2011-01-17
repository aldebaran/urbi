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

#include "sensor2.hh"

UStart(sensor2);

sensor2::sensor2(std::string s)
  : UObject(s)
{
  val = new UVar("sensor.val");

  UBindFunction(sensor2, init);
  UBindFunction(sensor2, setVal);
  UNotifyChange(*val, &sensor2::newval);
}

int
sensor2::init()
{
  return 0;
}

UReturn
sensor2::newval(UVar& v)
{
  std::cout << "sensor.val notifychange: " << (int)v << std::endl;
  return 0;
}

void
sensor2::setVal(int)
{
}

UReturn
sensor2::getval(UVar& v)
{
  v = 666;
  return 0;
}
