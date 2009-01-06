#include "uaccess.hh"
#include <iostream>
using namespace urbi;

UStart (uaccess);

uaccess::uaccess (const std::string& s)
  : UObject (s)
{
  UBindFunction (uaccess, init);
  UBindVar (uaccess, val);
  UOwned (val);
  UNotifyAccess (val, &uaccess::newval);
  UNotifyChange  (val, &uaccess::changed);
}

int
uaccess::init ()
{
  val = 0;
  return 0;
}

UReturn
uaccess::newval (UVar& v)
{
  static int value = 0;
  value++;
  std::cerr << "uaccess::newval: value = " << value << std::endl;
  v = value;
  return 0;
}

UReturn
uaccess::changed (UVar& v)
{
  std::cout <<  (int)v << "\n";
  return 0;
}
