#include <iostream>
#include <libport/debug.hh>
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
  GD_FERROR("uchange::newval: %s", ((int) v));
  return 0;
}
