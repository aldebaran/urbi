/**
 ** \file ast/loc.hh
 ** \brief Definition of ast::loc.
 */

#ifndef AST_LOC_HH
# define AST_LOC_HH

# include <libport/symbol.hh>

# include <parser/location.hh>

# define __HERE__ (::ast::loc(new libport::Symbol(__FILE__), __LINE__))

namespace ast
{
  typedef yy::location loc;
}

#endif // !AST_LOC_HH
