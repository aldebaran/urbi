/**
 ** \file ast/visitor.hxx
 ** \brief Definition of ast::Visitor.
 */

#ifndef AST_VISITOR_HXX
# define AST_VISITOR_HXX

# include <ast/visitor.hh>

namespace ast
{
  template < template <typename> class Const >
  GenVisitor<Const>::~GenVisitor ()
  {
  }

  template < template <typename> class Const >
  inline
  void
  GenVisitor<Const>::operator() (libport::shared_ptr<typename Const<Ast>::type> e)
  {
    if (e)
      e->accept(*this);
  }

} // namespace ast

#endif // !AST_VISITOR_HXX
