#include <iostream>
#include "uchange.hh"

using namespace urbi;

UStart (uchange);

uchange::uchange (const std::string& s)
  : UObject (s)
{
  UBindFunction (uchange, init);
}

int
uchange::init ()
{
  val = new UVar ("uaccess.val");
  UNotifyChange (*val, &uchange::newval);
  return 0;
}

UReturn
uchange::newval (UVar& v)
{
  std::cerr << "uchange::newval: " << (int) v << std::endl;
  return 0;
}
