#ifndef AST_DECLARATIONS_TYPE_HH
# define AST_DECLARATIONS_TYPE_HH

# include <iosfwd>
# include <vector>

# include <ast/local-declaration.hh>

namespace ast
{

  /// List of declarations
  typedef std::vector<rLocalDeclaration> local_declarations_type;

  /// Separated by commas.
  std::ostream&
  operator<<(std::ostream& o, const ast::local_declarations_type& ss);

}

#endif // ! AST_DECLARATIONS_TYPE_HH
