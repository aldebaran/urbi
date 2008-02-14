/**
 ** \file ast/flow-exception.cc
 ** \brief Implementation of ast::FlowException.
 */

#include "ast/flow-exception.hh"

namespace ast
{
  typedef boost::error_info<struct tag_location, loc> location_info;
  typedef boost::error_info<struct tag_rObject, object::rObject> rObject_info;

  FlowException::FlowException(const loc& location)
    : boost::exception ()
  {
    *this << location_info (location);
  }

  loc
  FlowException::location_get () const
  {
    return *boost::get_error_info<location_info> (*this);
  }

  BreakException::BreakException(const loc& location)
    : FlowException(location)
  {
  }

  ReturnException::ReturnException (const loc& location,
				    object::rObject result)
    : FlowException(location)
  {
    *this << rObject_info (result);
  }

  object::rObject ReturnException::result_get() const
  {
    return *boost::get_error_info<rObject_info> (*this);
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
