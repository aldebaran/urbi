/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <iostream>
#include <urbi/kernel/utypes.hh>

std::ostream&
operator<< (std::ostream& o, UErrorValue v)
{
  switch (v)
  {
#define CASE(V) case V: o << #V; break;
    CASE(USUCCESS);
    CASE(UFAIL);
#undef CASE
  }
  return o;
}
