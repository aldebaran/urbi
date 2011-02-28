/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/debug.hh>
#include <iostream>
#include "generic.hh"

GD_CATEGORY(Test.Generic);
UStart (generic);

generic::generic(const std::string& s)
  : UObject(s)
{
  UBindVar(generic, val);
  UBindFunction(generic, init);
  UBindFunction(generic, foo);
  UBindFunction(generic, inc);

  UNotifyChange(val, &generic::newval);
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
generic::init()
{
  return 0;
}

int
generic::foo(int x)
{
  GD_FINFO_DEBUG("foo(%s)", x);
  val = x;
  return x+1;
}

IF_VOID(void, int)
generic::inc()
{
  GD_INFO_DEBUG("inc");
  // Yeah, not nicely written...  Study the UVar interface to rewrite
  // this cleanly.
  val = (int) val + 1;
  IF_VOID(, return 666);
}

UReturn
generic::newval(UVar& v)
{
  LIBPORT_USE(v);
  GD_FINFO_DEBUG("newval: %s", (int) v);
  return 0;
}
