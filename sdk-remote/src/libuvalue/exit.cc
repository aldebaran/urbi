/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/exit.hh>

namespace urbi
{
  Exit::Exit(int error, const std::string& message)
      : error_(error)
      , msg_(message)
  {}

  Exit::~Exit() throw ()
  {}

  const char* Exit::what() const throw ()
  {
    return msg_.c_str();
  }

  int Exit::error_get() const
  {
    return error_;
  }
}
