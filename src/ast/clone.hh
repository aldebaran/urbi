/**
 ** \file ast/clone.hh
 ** \brief Declaration of ast::Clone.
 */

#ifndef AST_CLONE_HH
# define AST_CLONE_HH

# include "ast/fwd.hh"

namespace ast
{
  template <typename AstNode>
  AstNode* clone (const AstNode& ast);
}

# include "ast/clone.hxx"

#endif // !AST_CLONE_HH
