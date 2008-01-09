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

  std::ostream& operator<<(std::ostream& o, flow_exception_kind k)
  {
    switch (k)
    {
      case break_exception: return o << "break";
      default: return o;
    }
  }
} // namespace ast
