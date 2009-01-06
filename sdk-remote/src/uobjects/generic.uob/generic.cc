// #define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"
#include <iostream>
#include "generic.hh"

UStart (generic);

generic::generic (const std::string& s)
  : UObject (s)
{
  UBindVar (generic, val);
  UBindFunction (generic, init);
  UBindFunction (generic, foo);
  UBindFunction (generic, inc);

  UNotifyChange (val, &generic::newval);
  PING();
  // This means that the group "generics" will be created automatically.
  UAutoGroup();
}

int
generic::init ()
{
  PING();
  return 0;
}

int
generic::foo (int x)
{
  PING();
  val = x;
  return x+1;
}

IF_VOID(void, int)
generic::inc ()
{
  PING();
  // Yeah, not nicely written...  Study the UVar interface to rewrite
  // this cleanly.
  val = (int) val + 1;
  IF_VOID(, return 666);
}

UReturn
generic::newval (UVar& v)
{
  std::cerr << "generic:newval: " << (int) v;
  return 0;
}
