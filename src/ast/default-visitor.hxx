/**
 ** \file ast/default-visitor.hxx
 ** \brief Implementation for ast/default-visitor.hh.
 */

#ifndef AST_DEFAULT_VISITOR_HXX
# define AST_DEFAULT_VISITOR_HXX

# include "ast/all.hh"
# include "ast/default-visitor.hh"

namespace ast
{

  template < template<typename> class Const >
  GenDefaultVisitor<Const>::GenDefaultVisitor () :
    GenVisitor<Const> ()
  {
  }

  template < template<typename> class Const >
  GenDefaultVisitor<Const>::~GenDefaultVisitor ()
  {
  }

  template < template<typename> class Const >
  template<class E>
  void
  GenDefaultVisitor<Const>::operator() (E* e)
  {
    e->accept (*this);
  }

  template < template<typename> class Const >
  void
  GenDefaultVisitor<Const>::operator() (typename Const<Ast>::type& e)
  {
    super_type::operator() (e);
  }

  template < template<typename> class Const >
  void
  GenDefaultVisitor<Const>::operator() (typename Const<IntExpr>::type&)
  {
  }

  template < template<typename> class Const >
  void
  GenDefaultVisitor<Const>::operator() (typename Const<AssignExp>::type& e)
  {
    e.var_get ().accept (*this);
    e.exp_get ().accept (*this);
  }

  template < template<typename> class Const >
  void
  GenDefaultVisitor<Const>::operator() (typename Const<Exp>::type& e)
  {
  }

} // namespace ast

#endif // !AST_DEFAULT_VISITOR_HXX
