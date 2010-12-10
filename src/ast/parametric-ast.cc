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
 ** \file ast/parametric-ast.cc
 ** \brief Implementation of ast::ParametricAst.
 */

#include <ast/cloner.hxx>
#include <ast/parametric-ast.hh>
#include <ast/print.hh>
#include <ast/factory.hh>

#include <parser/parse.hh>

#include <rewrite/rewrite.hh>

namespace ast
{

  /*----------------.
  | ParametricAst.  |
  `----------------*/

  ParametricAst::ParametricAst(const char* s, const loc& l, bool desugar)
    : ast_(parser::parse_meta(s, l))
  {
    /*  Simplify the result as much as possible
     *
     *  Typically, even when parsing "42", the result is included in a
     *  useless singleton nary and a statement.
     */

    ast::rConstNary nary = ast_.unchecked_cast<const ast::Nary>();
    if (desugar)
      nary = rewrite::rewrite(nary);
    passert("ParametricAst result is not a Nary", nary);

    // Remove useless nary and statement if there's only one child.
    if (nary->children_get().size() == 1)
    {
      ast::rConstStmt stmt =
        nary->children_get().front().unsafe_cast<const ast::Stmt>();
      passert("ParametricAst's Nary child isn't a statement", stmt);
      ast_ = stmt->expression_get();
    }
  }

  ParametricAst::~ParametricAst()
  {
  }

  /*------------------.
  | ParameterizedAst.  |
  `------------------*/

  ParameterizedAst::ParameterizedAst(const ParametricAst& s)
    : Cloner()
# define CONSTRUCT(Iter, None, Type)                                    \
      , BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Type), _map_type)        \
        (BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2, 1, Type)))
      URBI_PARAMETERIZED_AST_FOREACH(CONSTRUCT)
#undef CONTSTRUCT
    , ast_(s)
    , effective_location_()
    , count_(0)
  {
  }

  ParameterizedAst::~ParameterizedAst()
  {
    passert(*this, empty());
  }

  void
  ParameterizedAst::visit(const ast::MetaArgs* e)
  {
    ast::rLValue lvalue = recurse(e->lvalue_get());
    lvalue->call()->arguments_set(exps_map_type::take_(e->id_get () - 1));
    result_ = lvalue;
  }

  void
  ParameterizedAst::visit(const ast::MetaCall* e)
  {
    libport::Symbol name = id_map_type::take_(e->id_get () - 1);
    result_ = new ast::Call(e->location_get(),
			    maybe_recurse_collection(e->arguments_get()),
                            recurse(e->target_get()), name);
  }

  void
  ParameterizedAst::visit(const ast::MetaExp* e)
  {
    result_ = exp_map_type::take_(e->id_get () - 1);
    aver(result_);
  }

  void
  ParameterizedAst::visit(const ast::MetaId* e)
  {
    libport::Symbol name = id_map_type::take_(e->id_get () - 1);
    result_ = ast::Factory::make_call(e->location_get(), name);
  }

  void
  ParameterizedAst::visit(const ast::MetaLValue* e)
  {
    result_ = exp_map_type::take_(e->id_get () - 1);
    aver(result_.unsafe_cast<const ast::LValue>());
    aver(result_);
  }

  bool
  ParameterizedAst::empty() const
  {
    return true
# define TEST(Iter, None, Type)                                         \
      && BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Type),                  \
                      _map_type)::empty_()
      URBI_PARAMETERIZED_AST_FOREACH(TEST)
# undef TEST
      ;
  }

  void
  ParameterizedAst::reset()
  {
    passert(*this, empty());
#ifndef NDEBUG
    unique_.clear();
#endif
    effective_location_.initialize(0);
    count_ = 0;
  }

  void
  ParameterizedAst::clear()
  {
# define CLEAR(Iter, None, Type)                                        \
    BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Type), _map_type)::clear_();
      URBI_PARAMETERIZED_AST_FOREACH(CLEAR)
# undef CLEAR
    reset();
  }

  std::ostream&
  ParameterizedAst::dump(std::ostream& o) const
  {
    return o
      << "ParameterizedAst:"
      << libport::incendl
      << "Ast:"
      << libport::incendl << *ast_.get() << libport::decendl
# define PRINT(Iter, None, Type)                                        \
      << static_cast<const BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 1, Type), \
                                        _map_type)&>(*this)
      URBI_PARAMETERIZED_AST_FOREACH(PRINT)
# undef PRINT
      << libport::decendl;
  }


  /*--------------------------.
  | Free-standing functions.  |
  `--------------------------*/

  rExp
  exp(ParameterizedAst& a)
  {
    return a.result<Exp>();
  }

  std::ostream&
  operator<< (std::ostream& o, const ParameterizedAst& a)
  {
    return a.dump(o);
  }

  ParameterizedAst&
  ParameterizedAst::operator% (libport::intrusive_ptr<ast::Exp> t)
  {
    // Strangely, the exp_map_type qualification is required
    // here. Factoring the two % operators in a template method is
    // thus impossible.
    exp_map_type::append_(count_, t);
    // Compute the location of the source text we used.
    if (!effective_location_.begin.filename)
      effective_location_ = t->location_get();
    else
      effective_location_ = effective_location_ + t->location_get();
    return *this;
  }

  ParameterizedAst&
  ParameterizedAst::operator% (libport::Symbol id)
  {
    id_map_type::append_(count_, id);
    return *this;
  }

  ParameterizedAst&
  ParameterizedAst::operator% (ast::exps_type* exps)
  {
#ifndef NDEBUG
    passert(libport::deref << exps, unique_(exps));
#endif
    exps_map_type::append_(count_, exps);
    return *this;
  }

} // namespace ast
