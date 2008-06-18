/// \file uparser.cc

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <cassert>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include <libport/foreach.hh>

#include <ast/nary.hh>
#include <ast/print.hh>

#include <parser/parser-impl.hh>
#include <parser/parser-utils.hh>
#include <parser/tweast.hh>
#include <parser/utoken.hh>

#include <kernel/server-timer.hh>

namespace parser
{

  /*-------------.
  | ParserImpl.  |
  `-------------*/

  ParserImpl::ParserImpl()
    : tweast_ (0),
      loc_(),
      synclines_(),
      result_(0)
  {
  }

  void
  ParserImpl::parse_ (std::istream& source)
  {
    TIMER_PUSH("parse");
    passert(*result_, !result_.get());
    result_.reset(new ParseResult);
    yyFlexLexer scanner;
    scanner.switch_streams(&source, 0);
    parser_type p(*this, scanner);
    p.set_debug_level (!!getenv ("YYDEBUG"));
    ECHO("====================== Parse begin");
    result_->status = p.parse();
    ECHO("====================== Parse end: " << res);
    TIMER_POP("parse");
  }

  parse_result_type
  ParserImpl::parse(const std::string& s)
  {
    if (getenv("PARSE"))
      LIBPORT_ECHO("Parsing: " << s);
    std::istringstream is(s);
    parse_(is);
    if (getenv("PARSE"))
      LIBPORT_ECHO("Result: " << *result_);
    return result_;
  }

  parse_result_type
  ParserImpl::parse (Tweast& t)
  {
    // Recursive calls are forbidden.  If we want to relax this
    // constraint, note that we also need to save and restore other
    // member changed during the parsing, such as warnings_ and
    // errors_.  But it is simpler to recurse with the standalone
    // parse functions.
    passert(tweast_, !tweast_);
    tweast_ = &t;
    std::istringstream is(t.input_get());
    parse_(is);
    tweast_ = 0;
    return result_;
  }

  parse_result_type
  ParserImpl::parse_file (const std::string& fn)
  {
    std::ifstream f(fn.c_str());
    if (!f.good())
    {
      // Return an error instead of creating a valid empty ast.
      result_.reset(new ParseResult);
      result_->status = 1;
    }
    else
    {
      // A location pointing to it.
      location_type loc;
      loc.initialize(new libport::Symbol(fn));

      // Exchange with the current location so that we can restore it
      // afterwards (when reading the input flow, we want to be able to
      // restore the cursor after having handled a load command).
      std::swap(loc, loc_);
      ECHO("Parsing file: " << fn);
      parse_(f);
      std::swap(loc, loc_);
    }
    return result_;
  }

  void
  ParserImpl::error(const location_type& l, const std::string& msg)
  {
    result_->error(message_format(l, msg));
  }

  void
  ParserImpl::warn(const location_type& l, const std::string& msg)
  {
    result_->warn(message_format(l, msg));
  }

}
