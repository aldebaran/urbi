/**
 ** \file ast/clone.hxx
 ** \brief Inline implementation of clone().
 */

#ifndef AST_CLONE_HXX
# define AST_CLONE_HXX

# include <boost/static_assert.hpp>
# include <boost/type_traits/is_base_of.hpp>

# include "ast/clone.hh"
# include "ast/cloner.hh"

namespace ast
{

  template <typename AstNode>
  inline
  AstNode*
  clone (const AstNode& ast)
  {
    // Let cpp authors rot in hell.
#define COMMA ,
    BOOST_STATIC_ASSERT(boost::is_base_of<Ast COMMA AstNode>::value);
    Cloner cloner;
    cloner(ast);
    AstNode* res = dynamic_cast<AstNode*>(cloner.result_get());
    assert (res);
    return res;
  }

  template <typename AstNode>
  inline
  typename boost::remove_const<AstNode>::type*
  clone (AstNode* ast)
  {
    return ast ? clone(*ast) : 0;
  }
}

#endif // !AST_CLONE_HXX
