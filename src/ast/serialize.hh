#ifndef AST_SERIALIZE_HH
# define AST_SERIALIZE_HH

# include <ast/ast.hh>
# include <urbi/export.hh>

namespace ast
{
  void URBI_SDK_API
  serialize(rConstAst ast, std::ostream& output);

  rAst URBI_SDK_API
  unserialize(std::istream& output);
}

#endif
