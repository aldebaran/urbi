/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include <object/urbi-exception.hh>

namespace object
{
  UrbiException::UrbiException(const std::string& msg)
    : kernel::exception()
    , displayed_(false)
  {
    msg_ = msg;
  }

  UrbiException::UrbiException(const std::string& msg,
                               const ast::loc& loc)
    : kernel::exception()
    , displayed_(false)
  {
    msg_ = msg;
    location_ = loc;
  }

  UrbiException::UrbiException(const std::string& msg,
                               const libport::Symbol fun)
    : kernel::exception()
    , msg_(fun.empty() ? msg : fun.name_get() + ": " + msg)
    , function_(fun)
    , displayed_(false)
  {
  }

  UrbiException::~UrbiException() throw ()
  {
  }

  std::string
  UrbiException::what () const throw ()
  {
    return msg_get ();
  }

} // namespace object
