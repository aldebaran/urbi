/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include <object/symbols.hh>
#include <object/urbi-exception.hh>
#include <runner/call.hh>

namespace object
{

  /*------------.
  | MyException |
  `------------*/

  MyException::MyException(rObject value, const call_stack_type& bt)
    : value_(value)
    , bt_(bt)
  {}

  rObject
  MyException::value_get()
  {
    return value_;
  }

  const call_stack_type&
  MyException::backtrace_get()
  {
    return bt_;
  }

  /*--------------.
  | UrbiException |
  `--------------*/

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
