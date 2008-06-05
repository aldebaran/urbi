/**
 ** \file parser/tweast.cc
 ** \brief Implementation of parser::Tweast.
 */

#include <ast/call.hh>
#include <ast/exp.hh>
#include <ast/print.hh>

#include <parser/tweast.hh>

namespace parser
{

  unsigned Tweast::count_ = 0;

  Tweast::Tweast()
    : MetavarMap<ast::rCall>::MetavarMap("call")
    , MetavarMap<ast::rExp>::MetavarMap("exp")
    , MetavarMap<ast::exps_type*>::MetavarMap("exps")
    , MetavarMap<ast::symbols_type*>::MetavarMap("formals")
    , input_()
#ifndef NDEBUG
    , unique_()
#endif
  {
  }

  Tweast::Tweast (const std::string& str)
    : MetavarMap<ast::rCall>::MetavarMap("call")
    , MetavarMap<ast::rExp>::MetavarMap("exp")
    , MetavarMap<ast::exps_type*>::MetavarMap("exps")
    , MetavarMap<ast::symbols_type*>::MetavarMap("formals")
    , input_(str)
#ifndef NDEBUG
    , unique_()
#endif
  {
  }

  Tweast::~Tweast ()
  {
  }

  // Yes, this is ugly and shows the limitations of our approach (by
  // repeated inheritance).  Don't know how to make it cuter though.
  Tweast&
  Tweast::operator<< (Tweast& t)
  {
    // Steal the input string.
    input_ << t.input_.str();
    // Steal the contents.
    MetavarMap<ast::rCall>
      ::insert_(static_cast<MetavarMap<ast::rCall>&>(t));
    MetavarMap<ast::rExp>
      ::insert_(static_cast<MetavarMap<ast::rExp>&>(t));
    MetavarMap<ast::exps_type*>
      ::insert_(static_cast<MetavarMap<ast::exps_type*>&>(t));
    MetavarMap<ast::symbols_type*>
    ::insert_(static_cast<MetavarMap<ast::symbols_type*>&>(t));
    return *this;
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
      << static_cast<const MetavarMap<ast::rCall>&>(*this)
      << static_cast<const MetavarMap<ast::rExp>&>(*this)
      << static_cast<const MetavarMap<ast::exps_type*>&>(*this)
      << static_cast<const MetavarMap<ast::symbols_type*>&>(*this)
      ;
  }

  std::ostream&
  operator<< (std::ostream& o, const Tweast& in)
  {
    return in.dump (o);
  }

}
