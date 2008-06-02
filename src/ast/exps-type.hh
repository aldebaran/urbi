#ifndef AST_EXPS_TYPE_HH
# define AST_EXPS_TYPE_HH

# include <deque>
# include <iosfwd>

# include <ast/fwd.hh>

namespace ast
{

  /// List of expressions, for List, Nary, Call etc.
  typedef std::deque<rExp> exps_type;

  /// Separated by commas.
  std::ostream& operator<<(std::ostream& o, const ast::exps_type& ss);

}

#endif // ! AST_EXPS_TYPE_HH
