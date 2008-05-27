/**
 ** \file ast/parametric-ast.cc
 ** \brief Implementation of ast::ParametricAst.
 */

#include "ast/parametric-ast.hh"
#include "ast/print.hh"

#include "parser/parse.hh"
#include "parser/parse-result.hh"

namespace ast
{

  ParametricAst::ParametricAst(const std::string& s)
    : exp_map_type("exp")
    , ast_(parser::parse(s)->ast_xtake())
    , count_(0)
  {
  }

  ParametricAst::~ParametricAst()
  {
    passert(*this, empty());
  }

  void
  ParametricAst::visit (ast::rConstMetaExp e)
  {
    result_ = parser::MetavarMap<ast::rExp>::take_(e->id_get () - 1);
  }

  bool
  ParametricAst::empty() const
  {
    return exp_map_type::empty_();
  }

  void
  ParametricAst::clear()
  {
    passert(*this, empty());
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
      << libport::decendl;
  }


  /*--------------------------.
  | Free-standing functions.  |
  `--------------------------*/

  rExp
  exp (ParametricAst& a)
  {
    return a.result<Exp>();
  }

  std::ostream&
  operator<< (std::ostream& o, const ParametricAst& a)
  {
    return a.dump (o);
  }

} // namespace ast
