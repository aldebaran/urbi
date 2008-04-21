/**
 ** \file ast/loc.hh
 ** \brief Definition of ast::loc.
 */

#ifndef AST_LOC_HH
# define AST_LOC_HH

# include <libport/symbol.hh>

# include "parser/location.hh"

namespace ast
{
  typedef yy::location loc;
}

#endif // !AST_LOC_HH
