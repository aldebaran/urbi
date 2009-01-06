#include <iostream>
//#define ENABLE_DEBUG_TRACES
//#include "libport/compiler.hh"
#include "uaccess.hh"
using namespace urbi;

UStart (uaccess);

uaccess::uaccess (const std::string& s)
  : UObject (s)
{
  UBindFunction (uaccess, init);
  UBindVar (uaccess, val);
  // This uobject is used for non uowned tests
  // UOwned(val);
  // UNotifyAccess (val, &uaccess::newval);
  UNotifyChange  (val, &uaccess::changed);
}

int
uaccess::init ()
{
  val = 0;
  //  ECHO("val = " << val);
  return 0;
}

UReturn
uaccess::newval (UVar& v)
{
  static int value = 0;
  value++;
  v = value;
  //  ECHO("v = " << v << ", val = " << val);
  return 0;
}

UReturn
uaccess::changed (UVar&)
{
  return 0;
}
