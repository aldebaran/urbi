/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/uobject.hh>

class Test : public urbi::UObject
{
public:
  Test(const std::string&s)
    : urbi::UObject(s)
  {
    UBindFunction (Test, init);
  }

  int init()
  {
    std::cout << "[00000000:issue] *** init"<< std::endl;
    UBindThreadedFunction(Test, foo, urbi::LOCK_INSTANCE);
    return 0;
  }

  int foo(const std::string& s)
  {
    std::cout << "[00000000:issue] *** mmm sleepy..." << s << std::endl;
    sleep(1);
    std::cout << "[00000000:issue] *** awake!"<< std::endl;
    return 0;
  }

};

UStart(Test);
