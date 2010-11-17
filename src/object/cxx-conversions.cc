/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

 #include <urbi/object/cxx-conversions.hh>


namespace urbi
{
  namespace object
  {
    TypeError::TypeError(const rObject& expected)
      : expected_(expected)
    {}

    TypeError::~TypeError() throw()
    {}
  }
}
