/**
 ** \file parser/tweast.cc
 ** \brief Implementation of parser::Tweast.
 */

#include <sstream>

#include <libport/foreach.hh>

#include "ast/pretty-printer.hh"

#include "parser/tweast.hh"

namespace parser
{
  unsigned Tweast::count_ = 0;

  Tweast::Tweast()
    : MetavarMap<ast::Call>::MetavarMap("call"),
      MetavarMap<ast::Exp>::MetavarMap("exp"),
      input_(),
      unique_()
  {
  }

  Tweast::Tweast (const std::string& str)
    : MetavarMap<ast::Call>::MetavarMap("call"),
      MetavarMap<ast::Exp>::MetavarMap("exp"),
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
      << "Call map:"
      << libport::incendl << MetavarMap<ast::Call>::map_ << libport::decendl
      << "Exp map:"
      << libport::incendl << MetavarMap<ast::Exp>::map_ << libport::decendl
      << "Input string:"
      << libport::incendl << input_.str () << libport::decendl;
  }

  std::ostream&
  operator<< (std::ostream& ostr, const Tweast& in)
  {
    return in.dump (ostr);
  }

}
