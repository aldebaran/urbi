/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include <object/urbi-exception.hh>

namespace object
{
  UrbiException::UrbiException (const std::string& msg)
    : kernel::exception (),
    displayed_(false)
  {
    msg_ = msg;
  }

  UrbiException::UrbiException (const std::string& msg,
				const ast::loc& loc)
    : kernel::exception (),
    displayed_(false)
  {
    msg_ = msg;
    location_ = loc;
  }

  UrbiException::UrbiException (const std::string& msg,
				const std::string& fun)
    : kernel::exception (),
    displayed_(false)
  {
    if (fun.empty ())
      msg_ = msg;
    else
      msg_ = fun + ": " + msg;
    function_ = fun;
  }

  UrbiException::~UrbiException () throw ()
  {
  }

  std::string
  UrbiException::what () const throw ()
  {
    return msg_get ();
  }

} // namespace object
