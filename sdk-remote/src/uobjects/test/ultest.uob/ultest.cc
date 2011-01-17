/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/uobject.hh>

class ultest : public urbi::UObject
{
public:
  ultest(const std::string& name)
    : urbi::UObject(name)
  {
    UBindFunction(ultest, f);
  }

  int f()
  {
    // Problem here.
    new urbi::UList();
    return 0;
  }

};

UStart(ultest);
