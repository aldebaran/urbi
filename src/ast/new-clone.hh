/**
 ** \file ast/new-clone.hh
 ** \brief Declaration of ast::new_clone().
 */

#ifndef AST_NEW_CLONE_HH
# define AST_NEW_CLONE_HH

# include <boost/type_traits/remove_const.hpp>

namespace ast
{

  /// Clone the \a ast.
  /// \precondition ast derives from Ast.
  template <typename AstNode>
  AstNode* new_clone(const AstNode& ast);

  /// Clone \a *ast.
  /// Since ASTs are often handled as pointers, this is very
  /// handy.  \a ast == 0 is valid and returns 0 too.
  template <typename AstNode>
  typename boost::remove_const<AstNode>::type* new_clone(AstNode* ast);

}

# include "ast/new-clone.hxx"

#endif // !AST_NEW_CLONE_HH
