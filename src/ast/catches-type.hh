#ifndef AST_CATCHES_TYPE_HH
# define AST_CATCHES_TYPE_HH

# include <deque>
# include <iosfwd>

# include <ast/catch.hh>
# include <ast/fwd.hh>

namespace ast
{
  /// List of catches
  typedef std::deque<rCatch> catches_type;
}

#endif // ! AST_CATCHES_TYPE_HH
