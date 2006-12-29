//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/int-exp.hh
 ** \brief Declaration of ast::IntExp.
 */

#ifndef AST_INT_EXP_HH
# define AST_INT_EXP_HH

# include "ast/exp.hh"

namespace ast
{

  /// IntExp.
  class IntExp: public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
     public:
    /// Construct an IntExp node.
    IntExp (const location& location, int value);
    /// Destroy an IntExp node.
    virtual ~IntExp ();
    /** \} */

    /** \name Visitors entry point.
     ** \{ */
  public:
    /// Accept a const visitor \a v.
    virtual void accept (ConstVisitor& v) const;
    /// Accept a non-const visitor \a v.
    virtual void accept (Visitor& v);
    /** \} */

    /** \name Accessors.
     ** \{ */
  public:
    /// Return stored integer value.
    int value_get () const;
    /** \} */

  protected:
    /// Stored integer value.
    int value_;
  };

} // namespace ast

# include "ast/int-exp.hxx"

#endif // !AST_INT_EXP_HH
