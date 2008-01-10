/**
 ** \file ast/flow-exception.cc
 ** \brief Implementation of ast::FlowException.
 */

#include "ast/flow-exception.hh"

namespace ast
{
  FlowException::FlowException(const loc& location):
    location_(location)
  { }

  const loc& FlowException::location_get () const
  {
    return location_;
  }

  BreakException::BreakException(const loc& location):
    FlowException(location)
  { }

  ReturnException::ReturnException(const loc& location, object::rObject result):
    FlowException(location), result_(result)
  { }

  ReturnException::~ReturnException() throw ()
  { }

  object::rObject ReturnException::result_get()
  {
    return result_;
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
