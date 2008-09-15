/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of Exception
 */

#include <object/symbols.hh>
#include <object/urbi-exception.hh>
#include <runner/call.hh>

namespace object
{

  /*--------------.
  | UrbiException |
  `--------------*/

  UrbiException::UrbiException(rObject value, const call_stack_type& bt)
    : value_(value)
    , bt_(bt)
  {}

  rObject
  UrbiException::value_get()
  {
    return value_;
  }

  const call_stack_type&
  UrbiException::backtrace_get()
  {
    return bt_;
  }

  /*----------.
  | Exception |
  `----------*/

  Exception::Exception(const std::string& msg)
    : kernel::exception()
    , displayed_(false)
  {
    msg_ = msg;
  }

  Exception::Exception(const std::string& msg,
                               const ast::loc& loc)
    : kernel::exception()
    , displayed_(false)
  {
    msg_ = msg;
    location_ = loc;
  }

  Exception::Exception(const std::string& msg,
                               const libport::Symbol fun)
    : kernel::exception()
    , msg_(fun.empty() ? msg : fun.name_get() + ": " + msg)
    , function_(fun)
    , displayed_(false)
  {
  }

  Exception::~Exception() throw ()
  {
  }

  std::string
  Exception::what () const throw ()
  {
    return msg_get ();
  }

} // namespace object
