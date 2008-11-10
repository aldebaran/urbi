#include "uaccess.hh"
using namespace urbi;

UStart (uaccess);

uaccess::uaccess (const std::string& s)
  : UObject (s)
{
  UBindFunction (uaccess, init);
  UBindVar (uaccess, val);
  UNotifyAccess (val, &uaccess::newval);
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

  v = value;
  return 0;
}
