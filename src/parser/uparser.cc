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

#include <libport/foreach.hh>

#include "ast/pretty-printer.hh"

#include "server-timer.hh"

#include "ast/nary.hh"

#include "parser/tweast.hh"
#include "parser/utoken.hh"
#include "parser/uparser.hh"

namespace parser
{

  /*----------.
  | UParser.  |
  `----------*/

  UParser::UParser()
    : tweast_ (0),
      ast_ (0),
      errors_ (),
      warnings_ (),
      loc_(),
      synclines_()
  {
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
  UParser::parse (const std::string& command)
  {
    std::istringstream is (command);
    ECHO("Parsing string: ==================\n"
	 << loc_ << ":\n" << command
	 << "\n==================================");
    return parse_ (is);
  }

  int
  UParser::parse (Tweast& t)
  {
    // Recursive calls are forbidden.  If we want to relax this
    // constraint, note that we also need to save and restore other
    // member changed during the parsing, such as warnings_ and
    // errors_.  But it is simpler to recurse with the standalone
    // parse functions.
    passert(tweast_, !tweast_);
    tweast_ = &t;
    int res = parse (t.input_get());
    tweast_ = 0;

    // We need the parse errors now.
    std::copy(warnings_.begin(), warnings_.end(),
	      std::ostream_iterator<std::string>(std::cerr, "\n"));
    std::copy(errors_.begin(), errors_.end(),
	      std::ostream_iterator<std::string>(std::cerr, "\n"));
    return res;
  }

  int
  UParser::parse_file (const std::string& fn)
  {
    // A location pointing to it.
    location_type loc;
    loc.initialize (new libport::Symbol(fn));

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


  /*--------------------------.
  | Free-standing functions.  |
  `--------------------------*/

  UParser
  parse(const std::string& cmd)
  {
    UParser res;
    int err = res.parse (cmd);
    passert (err, !err);
    return res;
  }

  UParser::ast_type*
  parse(Tweast& t)
  {
    UParser p;
    int err = p.parse (t);
    passert (err, !err);
    return p.ast_xtake().release();
  }

  UParser
  parse_file(const std::string& file)
  {
    UParser res;
    int err = res.parse_file (file);
    passert (err, !err);
    return res;
  }

}
