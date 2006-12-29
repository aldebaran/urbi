//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/int-exp.cc
 ** \brief Implementation of ast::IntExp.
 */

#include "ast/visitor.hh"
#include "ast/int-exp.hh"

namespace ast
{

  IntExp::IntExp (const location& location, int value):
    Exp (location),
    value_ (value)
  { }

  IntExp::~IntExp ()
  {
  }

  void
  IntExp::accept (ConstVisitor& v) const
  {
    v (*this);
  }
  void
  IntExp::accept (Visitor& v)
  {
    v (*this);
  }
    

} // namespace ast

