#ifndef AST_DECLARATIONS_TYPE_HH
# define AST_DECLARATIONS_TYPE_HH

# include <deque>
# include <iosfwd>

# include <ast/declaration.hh>

namespace ast
{

  /// List of expressions, for List, Nary, Call etc.
  typedef std::deque<rDeclaration> declarations_type;

  /// Separated by commas.
  std::ostream& operator<<(std::ostream& o, const ast::declarations_type& ss);

}

#endif // ! AST_DECLARATIONS_TYPE_HH
