/// \file uparser.cc

//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <cstdlib>
#include <cassert>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <strstream>

#include <libport/foreach.hh>

#include "ast/pretty-printer.hh"

#include "server-timer.hh"

#include "ast/nary.hh"

#include "parser/tweast.hh"
#include "parser/utoken.hh"
#include "parser/uparser.hh"

/*----------.
| UParser.  |
`----------*/

UParser::UParser()
  : ast_ (0),
    errors_ (),
    warnings_ (),
    loc_(),
    synclines_()
{
  // The first column for locations is 1.
  loc_.begin.column = loc_.end.column = 1;
}

int
UParser::parse_ (std::istream& source)
{
  TIMER_PUSH("parse");
  passert(ast_, !ast_);
  yyFlexLexer scanner;
  scanner.switch_streams(&source, 0);
  parser_type p(*this, scanner);
  p.set_debug_level (!!getenv ("YYDEBUG"));
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
  ECHO("Parsing string: ==================\n"
       << loc_ << ":\n" << command
       << "\n==================================");
  return parse_ (mem_buff);
}

int
UParser::process (parser::Tweast& t)
{
  // We need to keep it to be able to get the variables.
  // FIXME: Non-reentrant.
  tweast_ = &t;
  int res = process (t.input_get());
  // We need the parse errors now.
  std::copy(warnings_.begin(), warnings_.end(),
	    std::ostream_iterator<std::string>(std::cerr, "\n"));
  std::copy(errors_.begin(), errors_.end(),
	    std::ostream_iterator<std::string>(std::cerr, "\n"));
  return res;
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
  ECHO("Parsing file: " << fn);
  int res = parse_(f);
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
