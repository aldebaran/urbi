#ifndef AST_CATCHES_TYPE_HH
# define AST_CATCHES_TYPE_HH

# include <iosfwd>
# include <vector>

# include <ast/catch.hh>
# include <ast/fwd.hh>

namespace ast
{
  /// List of catches.
  typedef std::vector<rCatch> catches_type;

  std::ostream& operator<<(std::ostream& o, const catches_type& c);
}

#endif // ! AST_CATCHES_TYPE_HH
