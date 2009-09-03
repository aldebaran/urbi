/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
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
