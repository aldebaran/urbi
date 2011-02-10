/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include "uaccess.hh"
#include <libport/debug.hh>
#include <iostream>
using namespace urbi;

UStart (uaccess);
GD_CATEGORY(Test.UAccess);

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
  GD_FINFO_DEBUG("newval: value = %s", value);
  v = value;
  return 0;
}

UReturn
uaccess::changed (UVar& v)
{
  std::cout <<  (int)v << "\n";
  return 0;
}
