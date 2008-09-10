/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include <object/symbols.hh>
#include <object/urbi-exception.hh>
#include <runner/call.hh>

namespace object
{
  /*-------------------.
  | UnhandledException |
  `-------------------*/

  UnhandledException::UnhandledException(object::rObject value,
                                         const call_stack_type& bt)
    : value_(value)
    , bt_(bt)
  {}

  std::string UnhandledException::what(runner::Runner& r)
  {
    return urbi_call(r, value_,
                     SYMBOL(asString))->as<object::String>()->value_get();
  }

  call_stack_type UnhandledException::backtrace()
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
