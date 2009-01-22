#ifndef AST_DOT_PRINT_HH
# define AST_DOT_PRINT_HH

# include <ostream>

# include <ast/fwd.hh>
# include <urbi/export.hh>

namespace ast
{
  void URBI_SDK_API dot_print(rConstAst ast, std::ostream& stream);
}

#endif
