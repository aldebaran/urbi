/**
 ** \file ast/parametric-ast.cc
 ** \brief Implementation of ast::ParametricAst.
 */

#include "ast/parametric-ast.hh"
#include "ast/pretty-printer.hh"
#include "parser/uparser.hh"

namespace ast
{

  namespace
  {
    parser::UParser::ast_type*
    parse(const std::string& s)
    {
      parser::UParser p;
      int err = p.parse (s);
      p.dump_errors();
      passert (err, !err);
      return p.ast_xtake().release();
    }
  }

  ParametricAst::ParametricAst(const std::string& s)
    : exp_map_type("exp")
    , ast_(parse(s))
    , count_(0)
  {
  }

  ParametricAst::~ParametricAst()
  {
    delete ast_;
  }

  void
  ParametricAst::operator() (const ast::MetaExp& e)
  {
    result_ = parser::MetavarMap<Exp>::take_(e.id_get () - 1);
  }

  void
  ParametricAst::clear()
  {
    passert(exp_map_type::map_, exp_map_type::empty_());
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

  Exp*
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
