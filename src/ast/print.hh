/**
 ** \file ast/print.hh
 ** \brief Definition of operator<< for Ast.
 */

#ifndef AST_PRINT_HH
# define AST_PRINT_HH

# include <iosfwd>

namespace ast
{

  class Ast;
  std::ostream& operator<< (std::ostream& o, const Ast& a);

} // namespace ast

#endif // !AST_PRINT_HH

