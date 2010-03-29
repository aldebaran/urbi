/*
 * Copyright (C) 2007-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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
  GenVisitor<Const>::operator() (ast_type e)
  {
    if (e)
      e->accept(*this);
  }

} // namespace ast

#endif // !AST_VISITOR_HXX
