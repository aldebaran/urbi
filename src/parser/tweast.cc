/**
 ** \file parser/tweast.cc
 ** \brief Implementation of parser::Tweast.
 */

#include "ast/call.hh"
#include "ast/exp.hh"
#include "ast/print.hh"

#include "parser/tweast.hh"

namespace parser
{

  unsigned Tweast::count_ = 0;

  Tweast::Tweast()
    : MetavarMap<ast::Call>::MetavarMap("call")
    , MetavarMap<ast::Exp>::MetavarMap("exp")
    , MetavarMap<ast::exps_type>::MetavarMap("exps")
    , MetavarMap<ast::symbols_type>::MetavarMap("formals")
    , input_()
#ifndef NDEBUG
    , unique_()
#endif
  {
  }

  Tweast::Tweast (const std::string& str)
    : MetavarMap<ast::Call>::MetavarMap("call")
    , MetavarMap<ast::Exp>::MetavarMap("exp")
    , MetavarMap<ast::exps_type>::MetavarMap("exps")
    , MetavarMap<ast::symbols_type>::MetavarMap("formals")
    , input_(str)
#ifndef NDEBUG
    , unique_()
#endif
  {
  }

  Tweast::~Tweast ()
  {
  }

  std::string
  Tweast::input_get () const
  {
    return input_.str ();
  }

  std::ostream&
  Tweast::dump (std::ostream& o) const
  {
    return o
      << "Input string:"
      << libport::incendl << input_.str () << libport::decendl
      << static_cast<const MetavarMap<ast::Call>&>(*this)
      << static_cast<const MetavarMap<ast::Exp>&>(*this)
      << static_cast<const MetavarMap<ast::exps_type>&>(*this)
      << static_cast<const MetavarMap<ast::symbols_type>&>(*this)
      ;
  }

  std::ostream&
  operator<< (std::ostream& o, const Tweast& in)
  {
    return in.dump (o);
  }

}
