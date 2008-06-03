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
  /// \precondition AstNode derives from Ast.
  template <typename AstNode>
  libport::shared_ptr<AstNode> new_clone(libport::shared_ptr<const AstNode> ast);
  template <typename AstNode>
  libport::shared_ptr<AstNode> new_clone(libport::shared_ptr<AstNode> ast);

}

# include <ast/new-clone.hxx>

#endif // !AST_NEW_CLONE_HH
