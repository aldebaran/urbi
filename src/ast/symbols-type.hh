#ifndef AST_SYMBOLS_TYPE_HH
# define AST_SYMBOLS_TYPE_HH

# include <iostream>
# include <list>

# include <libport/fwd.hh> 

namespace ast
{
  /// Function formal arguments.
  typedef std::list<libport::Symbol*> symbols_type;
}

// We need argument dependant name lookup for symbols_type,
// which is actually a class of std: it is std::list<...>.
namespace std
{
  /// Separated by commas.
  std::ostream&
  operator<<(std::ostream& o, const ast::symbols_type& ss);
}

#endif // ! AST_SYMBOLS_TYPE_HH
