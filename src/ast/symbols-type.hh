#ifndef AST_SYMBOLS_TYPE_HH
# define AST_SYMBOLS_TYPE_HH

# include <iosfwd>
# include <vector>

# include <libport/fwd.hh>

namespace ast
{
  /// Function formal arguments.
  typedef std::vector<libport::Symbol> symbols_type;
}

// We need argument dependent name lookup for symbols_type,
// which is actually a class of std: it is std::vector<...>.
namespace std
{
  /// Separated by commas.
  std::ostream&
  operator<<(std::ostream& o, const ast::symbols_type& ss);
}

#endif // ! AST_SYMBOLS_TYPE_HH
