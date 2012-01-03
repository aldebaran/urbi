/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file ast/new-clone.hh
 ** \brief Declaration of ast::new_clone().
 */

#ifndef AST_NEW_CLONE_HH
# define AST_NEW_CLONE_HH

# include <boost/type_traits/remove_const.hpp>
# include <libport/intrusive-ptr.hh>

namespace ast
{

  /// Clone the \a ast.
  /// \precondition AstNode derives from Ast.
  template <typename AstNode>
  libport::intrusive_ptr<AstNode> new_clone(libport::intrusive_ptr<const AstNode> ast);
  template <typename AstNode>
  libport::intrusive_ptr<AstNode> new_clone(libport::intrusive_ptr<AstNode> ast);

}

# include <ast/new-clone.hxx>

#endif // !AST_NEW_CLONE_HH
