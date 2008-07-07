/**
 ** \file ast/error.hxx
 ** \brief Inline methods of ast::Error.
 */

#ifndef AST_ERROR_HXX
# define AST_ERROR_HXX

# include <ast/error.hh>

namespace ast
{
  inline
  std::ostream&
  operator<< (std::ostream& o, const Error& e)
  {
    return e.dump(o);
  }

} // namespace ast

#endif // !AST_ERROR_HXX
