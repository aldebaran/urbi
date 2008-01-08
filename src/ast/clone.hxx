/**
 ** \file ast/clone.hxx
 ** \brief Inline implementation of clone().
 */

#ifndef AST_CLONE_HXX
# define AST_CLONE_HXX

# include "ast/clone.hh"
# include "ast/cloner.hh"

namespace ast
{
  template <typename AstNode>
  inline
  AstNode*
  clone (const AstNode& ast)
  {
    Cloner cloner;
    cloner(ast);
    AstNode* res = dynamic_cast<AstNode*>(cloner.result_get());
    assert (res);
    return res;
  }
}

#endif // !AST_CLONE_HXX
