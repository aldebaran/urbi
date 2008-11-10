#include <iostream>
#include "remote.hh"

UStart (remote);

remote::remote (const std::string& s)
  : UObject (s)
{
  UBindVar (remote, toto);
  UBindFunction (remote, init);
  UBindFunction (remote, foo);

  val = new UVar ("remote.val");
  UNotifyChange (*val, &remote::newval);
}

int
remote::init ()
{
  return 0;
}

UReturn
remote::newval (UVar& v)
{
  std::cout << "remote.val=" << (int)v << std::endl;
  return 0;
}

int
remote::foo (int x)
{
  toto = x;
  return x+1;
}
