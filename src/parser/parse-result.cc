/// \file parser/parse-result.cc

#include <algorithm>
#include <iostream>
#include <iterator>

#include <libport/deref.hh>
#include <libport/indent.hh>
#include <libport/foreach.hh>
#include <libport/separator.hh>

#include <ast/nary.hh>
#include <ast/print.hh>

#include <parser/parse-result.hh>

namespace parser
{

  namespace
  {
    static
    std::ostream&
    operator<<(std::ostream& o,
               const std::list<std::string>& ms)
    {
      std::copy(ms.begin(), ms.end(),
                std::ostream_iterator<std::string>(o, "\n"));
      return o;
    }
  }

  ParseResult::ParseResult()
    : status(-1)
    , ast_(0)
    , errors_()
    , warnings_()
    , reported_(false)
  {
  }

  ParseResult::ParseResult(ParseResult& rhs)
    : status(rhs.status)
    , ast_(rhs.ast_) // The ast is stolen here.
    , errors_(rhs.errors_)
    , warnings_(rhs.warnings_)
    , reported_(rhs.reported_)
  {
    // It is now up to the most recent object to output the result.
    reported_ = true;
  }

  ParseResult::~ParseResult()
  {
    if (!reported_
        && (!errors_.empty() || !warnings_.empty()))
      dump_errors();
  }

  bool
  ParseResult::good() const
  {
    return (!status
            && ast_.get()
            && errors_.empty()
            && warnings_.empty());
  }

  std::ostream&
  ParseResult::dump (std::ostream& o) const
  {
    reported_ = true;
    return o
      << "Status:"
      << libport::incendl << status << libport ::decendl
      << "Ast:"
      << libport::incendl << libport::deref << *ast_ << libport ::decendl
      << "Errors: "
      << libport::incendl << errors_ << libport ::decendl
      << "Warnings: "
      << libport::incendl << warnings_ << libport ::decindent
      ;
  }

  /*------.
  | Ast.  |
  `------*/

  // Not in the *.hxx to avoid the #include "ast/nary.hh".
  void
  ParseResult::ast_reset(libport::shared_ptr<ast_type> ast)
  {
    ast_ = ast;
  }

  // Not in the *.hxx to avoid the #include "ast/nary.hh".
  void
  ParseResult::ast_set(libport::shared_ptr<ast_type> ast)
  {
    ast_ = ast;
  }

  // Not in the *.hxx to avoid the #include "ast/nary.hh".
  libport::shared_ptr<ParseResult::ast_type>
  ParseResult::ast_take()
  {
    return ast_;
  }

  // Not in the *.hxx to avoid the #include "ast/nary.hh".
  libport::shared_ptr<ParseResult::ast_type>
  ParseResult::ast_xtake()
  {
    assert(ast_.get());
    return ast_take();
  }


  /*-----------.
  | Messages.  |
  `-----------*/

  void
  ParseResult::dump_errors() const
  {
    std::cerr << errors_ << warnings_;
  }

  void
  ParseResult::process_errors(ast::Nary& target)
  {
    reported_ = true;
    foreach(const std::string& e, warnings_)
      target.message_push(e, "warning");
    warnings_.clear();

    // Errors handling
    if (!errors_.empty())
    {
      ast_.reset();
      foreach(const std::string& e, errors_)
	target.message_push(e, "error");
      errors_.clear();
    }
  }

}
