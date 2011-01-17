/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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
