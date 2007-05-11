/**
 ** \file ast/default-visitor.hh
 ** \brief Allow to visit the Abstract syntax tree from any node.
 */

#ifndef AST_DEFAULT_VISITOR_HH
# define AST_DEFAULT_VISITOR_HH

# include "ast/visitor.hh"

namespace ast
{
  /** \brief Just visit the whole Ast tree.
   **
   ** GenDefaultVisitor<CONSTNESS-SELECTOR> visits the whole Ast tree,
   ** but does nothing else. */
  template < template<typename> class Const >
  class GenDefaultVisitor : public GenVisitor<Const>
  {
  public:
    /// Super class type.
    typedef GenVisitor<Const> super_type;

    /** \name Ctor & dtor.
     ** \{ */
    /// Construct a Default Visitor.
    GenDefaultVisitor ();
    /// Destroy a GenDefaultVisitor.
    virtual ~GenDefaultVisitor ();
    /** \} */

    // Using `using super_type::operator()' here causes an ICE in ICC
    // 9.0.  We redefine this operator (delegating to GenVisitor's
    // operator()) as a workaround.
    virtual void operator() (typename Const<Ast>::type& e);

    template<class E> void operator() (E* e);

    /** \name Visit nodes.
     ** \{ */
    virtual void operator() (typename Const<AssignExp>::type&);
    virtual void operator() (typename Const<IntExp>::type&);
    virtual void operator() (typename Const<Var>::type&);
    /** \} */
  };

  /// Shorthand for a const visitor.
  typedef GenDefaultVisitor<misc::constify_traits> DefaultConstVisitor;
  /// Shorthand for a non const visitor.
  typedef GenDefaultVisitor<misc::id_traits> DefaultVisitor;

} // namespace ast

# include "ast/default-visitor.hxx"

#endif // !AST_DEFAULT_VISITOR_HH
