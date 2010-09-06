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
 ** \file ast/parametric-ast.hxx
 ** \brief Inline implementation of ast::ParametricAst.
 */

#ifndef AST_PARAMETRIC_AST_HXX
# define AST_PARAMETRIC_AST_HXX

# include <boost/static_assert.hpp>
# include <boost/type_traits/is_base_of.hpp>

# include <ast/parametric-ast.hh>
# include <ast/print.hh>

namespace ast
{

  /*----------------.
  | ParametricAst.  |
  `----------------*/

  inline
  const Ast*
  ParametricAst::get() const
  {
    return ast_.get();
  }

  /*------------------.
  | ParametrizedAst.  |
  `------------------*/

  template <typename T>
  inline
  libport::intrusive_ptr<T>
  ParameterizedAst::result()
  {
    BOOST_STATIC_ASSERT((boost::is_base_of<Ast, T>::value));
    operator()(ast_.get());
    if (!loc_)
      result_->location_set(effective_location_);
    reset();
    return assert_exp(result_.unsafe_cast<T>());
  }

  template <typename T>
  inline
  T
  ParameterizedAst::take(unsigned s) throw (std::range_error)
  {
    return parser::MetavarMap<T>::take_(s);
  }

} // namespace ast

#endif // !AST_PARAMETRIC_AST_HH
