/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file ast/nary-fwd.hh
 ** \brief Forward declarations of some AST classes
 */

#ifndef AST_NARY_FWD_HH
# define AST_NARY_FWD_HH

# include <libport/intrusive-ptr.hh>

namespace ast
{
  class Ast;
  typedef libport::intrusive_ptr<Ast> rAst;
  typedef libport::intrusive_ptr<const Ast> rConstAst;

  class Nary;
  typedef libport::intrusive_ptr<Nary> rNary;
  typedef libport::intrusive_ptr<const Nary> rConstNary;
}

#endif // AST_NARY_FWD_HH
