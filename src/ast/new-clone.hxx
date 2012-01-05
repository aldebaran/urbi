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
 ** \file ast/new-clone.hxx
 ** \brief Inline implementation of ast::new_clone().
 */

#ifndef AST_CLONE_HXX
# define AST_CLONE_HXX

# include <boost/static_assert.hpp>
# include <boost/type_traits/is_base_of.hpp>

# include <ast/cloner.hh>
# include <ast/new-clone.hh>

namespace ast
{

  template <typename T>
  inline
  libport::intrusive_ptr<T>
  new_clone (libport::intrusive_ptr<const T> ast)
  {
    BOOST_STATIC_ASSERT((boost::is_base_of<Ast, T>::value));
    Cloner cloner(true);
    cloner(ast.get());
    libport::intrusive_ptr<T> res = libport::unsafe_cast<T>(cloner.result_get());
    aver(res);
    return res;
  }

  template <typename T>
  inline
  libport::intrusive_ptr<T>
  new_clone (libport::intrusive_ptr<T> ast)
  {
    return new_clone(libport::intrusive_ptr<const T>(ast));
  }

}

#endif // !AST_CLONE_HXX
