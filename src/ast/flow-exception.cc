/**
 ** \file ast/flow-exception.cc
 ** \brief Implementation of ast::FlowException.
 */

#include "ast/flow-exception.hh"

namespace ast
{
  FlowException::FlowException(const loc& location)
    : kernel::exception ()
  {
    location_ = location;
  }

  BreakException::BreakException(const loc& location)
    : FlowException(location)
  {
  }

  ReturnException::ReturnException (const loc& location,
				    object::rObject result)
    : FlowException(location)
  {
    result_ = result;
  }

  std::ostream& operator<<(std::ostream& o, flow_exception_kind k)
  {
    switch (k)
    {
      case break_exception: return o << "break";
      case return_exception: return o << "return";
      default: return o;
    }
  }
} // namespace ast
