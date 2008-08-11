#ifndef AST_DECLARATIONS_TYPE_HH
# define AST_DECLARATIONS_TYPE_HH

# include <deque>
# include <iosfwd>

# include <ast/local-declaration.hh>

namespace ast
{

  /// List of declarations
  typedef std::deque<rLocalDeclaration> local_declarations_type;

  /// Separated by commas.
  std::ostream&
  operator<<(std::ostream& o, const ast::local_declarations_type& ss);

}

#endif // ! AST_DECLARATIONS_TYPE_HH
