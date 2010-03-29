/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file ast/print.hh
 ** \brief Definition of operator<< for Ast.
 */

#ifndef AST_PRINT_HH
# define AST_PRINT_HH

# include <ast/fwd.hh>
# include <urbi/export.hh>
# include <iosfwd>

namespace ast
{

  URBI_SDK_API std::ostream& operator<< (std::ostream& o, const Ast& a);

} // namespace ast

#endif // !AST_PRINT_HH
