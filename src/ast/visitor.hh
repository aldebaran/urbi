//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/visitor.hh
 ** \brief Definition of ast::Visitor.
 */


#ifndef AST_VISITOR_HH
# define AST_VISITOR_HH

# include <functional>
# include "ast/fwd.hh"
# include "misc/select-const.hh"

namespace ast
{

  /** \brief Root class of all Ast visitors.
   **
   ** GenVisitor<CONSTIFY> is the root class of all Ast visitors. */
  template < template <typename> class Const >
  class GenVisitor : public std::unary_function<Ast, void>
  {
    /** \name Ctor & dtor.
     ** \{ */
  public:
    /// Destroy a GenVisitor.
    virtual ~GenVisitor ();
    /** \} */

    /// The entry point: visit \a e.
    virtual void operator() (typename Const<ast::Ast>::type& e);

    virtual void operator() (typename Const<AssignExp>::type&) = 0;
    virtual void operator() (typename Const<IntExp>::type&) = 0;
    virtual void operator() (typename Const<Var>::type&) = 0;

/*
    virtual void operator() (typename Const<FunctionDecs>::type&) = 0;
    virtual void operator() (typename Const<VarDecs>::type&) = 0;
    virtual void operator() (typename Const<TypeDecs>::type&) = 0;
*/
  };

  /// Shorthand for a const visitor.
  typedef GenVisitor<misc::constify_traits> ConstVisitor;
  /// Shorthand for a non const visitor.
  typedef GenVisitor<misc::id_traits> Visitor;

} // namespace ast

# include "ast/visitor.hxx"

#endif // !AST_VISITOR_HH
