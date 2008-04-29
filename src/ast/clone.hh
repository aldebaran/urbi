/**
 ** \file ast/clone.hh
 ** \brief Declaration of ast::Clone.
 */

#ifndef AST_CLONE_HH
# define AST_CLONE_HH

# include <boost/type_traits/remove_const.hpp>

namespace ast
{

  /// Clone the \a ast.
  /// \precondition ast derives from Ast.
  template <typename AstNode>
  AstNode* clone(const AstNode& ast);

  /// Clone \a *ast.
  /// Since ASTs are often handled as pointers, this is very
  /// handy.
  /// \precondition  ast != 0
  template <typename AstNode>
  typename boost::remove_const<AstNode>::type* clone(AstNode* ast);

}

# include "ast/clone.hxx"

#endif // !AST_CLONE_HH
