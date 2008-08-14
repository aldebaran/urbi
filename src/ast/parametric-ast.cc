/**
 ** \file ast/parametric-ast.cc
 ** \brief Implementation of ast::ParametricAst.
 */

#include <ast/parametric-ast.hh>
#include <ast/print.hh>

#include <parser/parse.hh>
#include <parser/parse-result.hh>
#include <ast/cloner.hxx>

namespace ast
{

  ParametricAst::ParametricAst(const std::string& s)
    : exp_map_type("exp")
    , lvalue_map_type("lvalue")
    , id_map_type("id")
    , ast_(parser::parse(s)->ast_xget())
    , effective_location_()
    , count_(0)
  {
    /*  Simplify the result as much as possible
     *
     *  Typically, even when parsing "42", the result is included in a
     *  useless singleton nary and a statement.
     */

    // FIXME: Use a static cast
    ast::rConstNary nary = ast_.unsafe_cast<const ast::Nary>();
    passert("ParametricAst result is not a Nary", res);

    // Remove useless nary and statement if there.s only one child
    if (nary->children_get().size() == 1)
    {
      ast::rConstStmt stmt = nary->children_get().front().unsafe_cast<const ast::Stmt>();
      passert("ParametricAst's Nary child isn't a statement", res);
      ast_ = stmt->expression_get();
    }
  }

  ParametricAst::~ParametricAst()
  {
    passert(*this, empty());
  }

  void
  ParametricAst::visit(const ast::MetaExp* e)
  {
    result_ = exp_map_type::take_(e->id_get () - 1);
    assert(result_);
  }

  void
  ParametricAst::visit(const ast::MetaLValue* e)
  {
    result_ = lvalue_map_type::take_(e->id_get () - 1);
    assert(result_);
  }

  void
  ParametricAst::visit(const ast::MetaCall* e)
  {
    libport::Symbol name = id_map_type::take_(e->id_get () - 1);

    result_ = new ast::Call(e->location_get(),
                            recurse_collection(e->arguments_get()),
                            recurse(e->target_get()), name);
  }

  bool
  ParametricAst::empty() const
  {
    return exp_map_type::empty_()
      && lvalue_map_type::empty_()
      && id_map_type::empty_();
  }

  void
  ParametricAst::clear()
  {
    passert(*this, empty());
#ifndef NDEBUG
    unique_.clear();
#endif
    effective_location_.initialize(0);
    count_ = 0;
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
      << static_cast<const lvalue_map_type&>(*this)
      << static_cast<const id_map_type&>(*this)
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
    passert(libport::deref << t, unique_(t));
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
  ParametricAst::operator% (ast::rLValue t)
  {
#ifndef NDEBUG
    passert(libport::deref << t, unique_(t));
#endif
    lvalue_map_type::append_(count_, t);
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
#ifndef NDEBUG
    passert(libport::deref << id, unique_(id));
#endif
    id_map_type::append_(count_, id);
    return *this;
  }

} // namespace ast
