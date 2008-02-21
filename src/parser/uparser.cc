/// \file uparser.cc

//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <cstdlib>
#include <cassert>

#include <sstream>
#include <strstream>
#include <fstream>
#include <algorithm>
#include <string>

#include <libport/foreach.hh>

#include "server-timer.hh"
#include "uparser.hh"
#include "ast/nary.hh"

/*----------.
| UParser.  |
`----------*/

UParser::UParser()
  : ast_ (0),
    errors_ (),
    warnings_ (),
    scanner_ (),
    loc_()
{
  // The first column for locations is 1.
  loc_.begin.column = loc_.end.column = 1;
}

int
UParser::parse_ ()
{
  TIMER_PUSH("parse");
  ast_ = 0;

  parser_type p(*this);
#ifdef ENABLE_DEBUG_TRACES
  p.set_debug_level(true);
#else
  p.set_debug_level (!!getenv ("YYDEBUG"));
#endif
  ECHO("====================== Parse begin");
  int res = p.parse();
  ECHO("====================== Parse end: " << res);
  TIMER_POP("parse");
  return res;
}

int
UParser::process (const std::string& command)
{
  // It has been said Flex scanners cannot work with istrstream.
  std::istrstream mem_buff (command.c_str ());
  std::istream mem_input (mem_buff.rdbuf());
  scanner_.switch_streams(&mem_input, 0);
  ECHO("Parsing string: ==================\n"
       << loc_ << ":\n" << command
       << "\n==================================");
  return parse_ ();
}

ast::Ast*
UParser::ast_get ()
{
  if (hasError ())
    return 0;
  return ast_;
}

const ast::Ast*
UParser::ast_get () const
{
  if (hasError ())
    return 0;
  return ast_;
}

void
UParser::ast_set (ast::Ast* ast)
{
  ast_ = ast;
}

int
UParser::process_file (const std::string& fn)
{
  // A location pointing to it.
  location_type loc;
  loc.initialize (new libport::Symbol(fn));
  // The convention for the first column changed: make sure the first
  // column is column 1.
  loc.begin.column = loc.end.column = 1;

  // Exchange with the current location so that we can restore it
  // afterwards (when reading the input flow, we want to be able to
  // restore the cursor after having handled a load command).
  std::swap(loc, loc_);
  std::ifstream f (fn.c_str());
  if (!f.good())
    return 1; // Return an error instead of creating a valid empty ast.
  scanner_.switch_streams(&f, 0);
  ECHO("Parsing file: " << fn);
  int res = parse_();
  std::swap(loc, loc_);
  return res;
}

void
UParser::message_push(messages_type& msgs,
		      const yy::parser::location_type& l,
		      const std::string& msg)
{
  std::ostringstream o;
  o << "!!! " << l << ": " << msg;
  msgs.push_back(o.str());
}

void
UParser::process_errors(ast::Nary* target)
{
  foreach(const std::string& e, warnings_)
    target->message_push(e, "warning");
  warnings_.clear();

  // Errors handling
  if (!errors_.empty())
  {
    delete ast_;
    ast_ = 0;

    foreach(const std::string& e, errors_)
      target->message_push(e, "error");
    errors_.clear();
  }
}

void
UParser::error (const yy::parser::location_type& l, const std::string& msg)
{
  message_push(errors_, l, msg);
}

void
UParser::warn (const yy::parser::location_type& l, const std::string& msg)
{
  message_push(warnings_, l, msg);
}

bool
UParser::hasError () const
{
  return !errors_.empty();
}

std::string
UParser::error_get () const
{
  // precondition
  assert(hasError());
  return errors_.front();
}

bool
UParser::hasWarning () const
{
  return !warnings_.empty();
}

std::string
UParser::warning_get () const
{
  // precondition
  assert(hasWarning());
  return warnings_.front();
}

void
UParser::warning_pop ()
{
  warnings_.pop_front();
}

void
UParser::error_pop ()
{
  errors_.pop_front();
}
