#ifndef PARSER_AST_FACTORY_HXX
# define PARSER_AST_FACTORY_HXX

# include <parser/ast-factory.hh>

namespace parser
{

  /// To use to solve the ambiguities bw MetaVar::append_ and
  /// Tweast::append_ when we don't use exactly ast::rExp.
  inline
  ast::rExp
  ast_exp (ast::rExp e)
  {
    return e;
  }

}
#endif // !PARSER_AST_FACTORY_HXX
