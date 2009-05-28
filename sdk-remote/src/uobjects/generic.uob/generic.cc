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
#if FIXME
  // This feature is deprecated (it is not implemented in k2, and it
  // was considered not relevant).  So unless there are reason to
  // reenable it, don't use it.

  // This means that the group "generics" will be created
  // automatically.
  UAutoGroup();
#endif
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
  GD_FERROR("generic:newval: %s", ((int) v));
  return 0;
}
