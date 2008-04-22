/**
 ** \file parser/tweast.cc
 ** \brief Implementation of parser::Tweast.
 */

#include "ast/pretty-printer.hh"

#include "parser/tweast.hh"


namespace parser
{

  unsigned Tweast::count_ = 0;

  Tweast::Tweast()
    : MetavarMap<ast::Call>::MetavarMap("call"),
      MetavarMap<ast::Exp>::MetavarMap("exp"),
      MetavarMap<ast::symbols_type>::MetavarMap("formals"),
      input_(),
      unique_()
  {
  }

  Tweast::Tweast (const std::string& str)
    : MetavarMap<ast::Call>::MetavarMap("call"),
      MetavarMap<ast::Exp>::MetavarMap("exp"),
      MetavarMap<ast::symbols_type>::MetavarMap("formals"),
      input_(str),
      unique_()
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
  Tweast::dump (std::ostream& ostr) const
  {
    return ostr
      << "Input string:"
      << libport::incendl << input_.str () << libport::decendl
      << "Call map:"
      << libport::incendl << MetavarMap<ast::Call>::map_ << libport::decendl
      << "Exp map:"
      << libport::incendl << MetavarMap<ast::Exp>::map_ << libport::decendl
      << "symbols_type map:"
      << libport::incendl << MetavarMap<ast::symbols_type>::map_ << libport::decendl
      ;
  }

  std::ostream&
  operator<< (std::ostream& ostr, const Tweast& in)
  {
    return in.dump (ostr);
  }

}
