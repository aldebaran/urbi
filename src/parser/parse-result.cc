/// \file parser/parse-result.cc

#include <iostream>

#include <libport/indent.hh>
#include <libport/foreach.hh>
#include <libport/separator.hh>

#include "ast/nary.hh"
#include "ast/pretty-printer.hh"

#include "parser/parse-result.hh"

namespace parser
{

  namespace
  {
    static 
    std::ostream&
    operator<<(std::ostream& o, 
               const std::list<std::string>& ms)
    {
      return o << libport::separate(ms, libport::iendl);
    }
  }

  ParseResult::~ParseResult()
  {
  }

  std::ostream&
  ParseResult::dump (std::ostream& o) const
  {
    return o
      << "Status:"
      << libport::incendl << status << libport ::decendl
      << "Ast:"
      << libport::incendl << *ast_ << libport ::decendl
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
  ParseResult::ast_reset(ast_type* ast)
  {
    ast_.reset(ast);
  }

  // Not in the *.hxx to avoid the #include "ast/nary.hh".
  void
  ParseResult::ast_set(std::auto_ptr<ast_type> ast)
  {
    ast_ = ast;
  }

  // Not in the *.hxx to avoid the #include "ast/nary.hh".
  std::auto_ptr<ParseResult::ast_type>
  ParseResult::ast_take()
  {
    return ast_;
  }

  // Not in the *.hxx to avoid the #include "ast/nary.hh".
  std::auto_ptr<ParseResult::ast_type>
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
    std::cerr
      << errors_ << std::endl
      << warnings_ << std::endl;
  }

  void
  ParseResult::process_errors(ast::Nary& target)
  {
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

