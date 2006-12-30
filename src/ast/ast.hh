//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/ast.hh
 ** \brief Declaration of ast::Ast.
 */

#ifndef AST_AST_HH
# define AST_AST_HH

# include "ast/fwd.hh"
# include "ast/loc.hh"
# include "ast/ast.hh"

namespace ast
{

  /// Ast.
  class Ast
  {
    /** \name Ctor & dtor.
     ** \{ */
     public:
    /// Construct an Ast node.
    Ast (const loc& location);
    /// Destroy an Ast node.
    virtual ~Ast ();
    /** \} */

    /** \name Visitors entry point.
     ** \{ */
  public:
    /// Accept a const visitor \a v.
    virtual void accept (ConstVisitor& v) const = 0;
    /// Accept a non-const visitor \a v.
    virtual void accept (Visitor& v) = 0;
    /** \} */

    /** \name Accessors.
     ** \{ */
  public:
    /// Return scanner position information.
    const loc& location_get () const;
    /// Set scanner position information.
    void location_set (const loc&);
    /// Return parent node.
    const Ast& up_get () const;
    /// Set parent node.
    void up_set (Ast*);
    /** \} */

  protected:
    /// Scanner position information.
    loc location_;
    /// Parent node.
    Ast* up_;
  };

} // namespace ast

# include "ast/ast.hxx"

#endif // !AST_AST_HH
