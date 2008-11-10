#include "sensor2.hh"

UStart (sensor2);

sensor2::sensor2 (std::string s)
  : UObject (s)
{
  val = new UVar ("sensor.val");

  UBindFunction (sensor2, init);
  UBindFunction (sensor2, setVal);
  UNotifyChange (*val, &sensor2::newval);
}

int
sensor2::init ()
{
  return 0;
}

UReturn
sensor2::newval (UVar& v)
{
  std::cout <<  "sensor.val notifychange: " << (int)v << std::endl;
  return 0;
}

void
sensor2::setVal (int v)
{
}

UReturn
sensor2::getval (UVar& v)
{
  v = 666;
  return 0;
}
