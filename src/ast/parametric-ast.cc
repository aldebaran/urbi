/**
 ** \file ast/parametric-ast.cc
 ** \brief Implementation of ast::ParametricAst.
 */

#include <ast/cloner.hxx>
#include <ast/parametric-ast.hh>
#include <ast/print.hh>

#include <parser/ast-factory.hh>
#include <parser/parse.hh>
#include <parser/parse-result.hh>

namespace ast
{

  ParametricAst::ParametricAst(const std::string& s)
    : exp_map_type("exp")
    , id_map_type("id")
    , exps_map_type("exps")
    , ast_(parser::parse(s)->ast_xget())
    , effective_location_()
    , count_(0)
  {
    /*  Simplify the result as much as possible
     *
     *  Typically, even when parsing "42", the result is included in a
     *  useless singleton nary and a statement.
     */

    ast::rConstNary nary = ast_.unchecked_cast<const ast::Nary>();
    passert("ParametricAst result is not a Nary", nary);

    // Remove useless nary and statement if there.s only one child
    if (nary->children_get().size() == 1)
    {
      ast::rConstStmt stmt = nary->children_get().front().unsafe_cast<const ast::Stmt>();
      passert("ParametricAst's Nary child isn't a statement", stmt);
      ast_ = stmt->expression_get();
    }
  }

  ParametricAst::~ParametricAst()
  {
    passert(*this, empty());
  }

  void
  ParametricAst::visit(const ast::MetaArgs* e)
  {
    ast::rLValue lvalue = recurse(e->lvalue_get());
    lvalue->call()->arguments_set(exps_map_type::take_(e->id_get () - 1));
    result_ = lvalue;
  }

  void
  ParametricAst::visit(const ast::MetaCall* e)
  {
    libport::Symbol name = id_map_type::take_(e->id_get () - 1);

    result_ = new ast::Call(e->location_get(),
                            recurse_collection(e->arguments_get()),
                            recurse(e->target_get()), name);
  }

  void
  ParametricAst::visit(const ast::MetaExp* e)
  {
    result_ = exp_map_type::take_(e->id_get () - 1);
    assert(result_);
  }

  void
  ParametricAst::visit(const ast::MetaId* e)
  {
    libport::Symbol name = id_map_type::take_(e->id_get () - 1);

    result_ = parser::ast_call(e->location_get(), name);
  }

  void
  ParametricAst::visit(const ast::MetaLValue* e)
  {
    result_ = exp_map_type::take_(e->id_get () - 1);
    assert(result_.unsafe_cast<const ast::LValue>());
    assert(result_);
  }

  bool
  ParametricAst::empty() const
  {
    return exp_map_type::empty_()
      && id_map_type::empty_()
      && exps_map_type::empty_();
  }

  void
  ParametricAst::reset()
  {
    passert(*this, empty());
#ifndef NDEBUG
    unique_.clear();
#endif
    effective_location_.initialize(0);
    count_ = 0;
  }

  void
  ParametricAst::clear()
  {
    exp_map_type::clear_();
    id_map_type::clear_();
    exps_map_type::empty_();
    reset();
  }

  std::ostream&
  ParametricAst::dump(std::ostream& o) const
  {
    return o
      << "ParametricAst:"
      << libport::incendl
      << "Ast:"
      << libport::incendl << *ast_ << libport::decendl
      << static_cast<const exp_map_type&>(*this)
      << static_cast<const id_map_type&>(*this)
      << static_cast<const exps_map_type&>(*this)
      << libport::decendl;
  }


  /*--------------------------.
  | Free-standing functions.  |
  `--------------------------*/

  rExp
  exp(ParametricAst& a)
  {
    return a.result<Exp>();
  }

  std::ostream&
  operator<< (std::ostream& o, const ParametricAst& a)
  {
    return a.dump(o);
  }

  ParametricAst&
  ParametricAst::operator% (libport::shared_ptr<ast::Exp, true> t)
  {
#ifndef NDEBUG
    // Implicit are shared.
    passert(libport::deref << t,
            t.unsafe_cast<ast::Implicit>() || unique_(t.get()));
#endif

    // Strangely, the exp_map_type qualification is required
    // here. Factoring the two % operator in a template method is thus
    // impossible.
    exp_map_type::append_(count_, t);
    // Compute the location of the source text we used.
    if (!effective_location_.begin.filename)
      effective_location_ = t->location_get();
    else
      effective_location_ = effective_location_ + t->location_get();
    return *this;
  }

  ParametricAst&
  ParametricAst::operator% (libport::Symbol id)
  {
    id_map_type::append_(count_, id);
    return *this;
  }

  ParametricAst&
  ParametricAst::operator% (ast::exps_type* exps)
  {
#ifndef NDEBUG
    passert(libport::deref << exps, unique_(exps));
#endif
    exps_map_type::append_(count_, exps);
    return *this;
  }

} // namespace ast
