/**
 ** \file object/flow-exception.cc
 ** \brief Implementation of object::FlowException.
 */

#include "object/flow-exception.hh"

namespace object
{
  FlowException::FlowException(const ast::loc& location)
    : kernel::exception ()
  {
    location_ = location;
  }

  BreakException::BreakException(const ast::loc& location)
    : FlowException(location)
  {
  }

  ReturnException::ReturnException (const ast::loc& location,
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
    }
    pabort(k);
  }
} // namespace object
