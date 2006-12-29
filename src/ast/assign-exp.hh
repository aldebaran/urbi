//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/assign-exp.hh
 ** \brief Declaration of ast::AssignExp.
 */

#ifndef AST_ASSIGN_EXP_HH
# define AST_ASSIGN_EXP_HH

# include "ast/var.hh"
# include "ast/exp.hh"

namespace ast
{

  /// AssignExp.
  class AssignExp: public Exp
  {
    /** \name Ctor & dtor.
     ** \{ */
     public:
    /// Construct an AssignExp node.
    AssignExp (const location& location, Var* var, Exp* exp);
    /// Destroy an AssignExp node.
    virtual ~AssignExp ();
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
    /// Return reference to the affected variable.
    const Var& var_get () const;
    /// Return reference to the affected variable.
    Var& var_get ();
    /// Return assigned value.
    const Exp& exp_get () const;
    /// Return assigned value.
    Exp& exp_get ();
    /** \} */

  protected:
    /// Reference to the affected variable.
    Var* var_;
    /// Assigned value.
    Exp* exp_;
  };

} // namespace ast

# include "ast/assign-exp.hxx"

#endif // !AST_ASSIGN_EXP_HH
