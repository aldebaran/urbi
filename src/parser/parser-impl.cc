/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file parser/parser-impl.cc

#include <kernel/config.h> // YYDEBUG.
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <libport/echo.hh>
#include <libport/format.hh>

#include <libport/cassert>
#include <libport/cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include <libport/finally.hh>
#include <libport/foreach.hh>

#include <ast/nary.hh>
#include <ast/print.hh>

#include <parser/parser-impl.hh>
#include <parser/parser-utils.hh>
#include <parser/utoken.hh>

#include <kernel/server-timer.hh>

namespace parser
{

  /*-------------.
  | ParserImpl.  |
  `-------------*/

  static bool yydebug = getenv("URBI_PARSER");

  ParserImpl::ParserImpl()
    : loc_()
    , synclines_()
    , result_(0)
    , debug_(yydebug)
    , meta_(false)
    , factory_(new ast::Factory)
  {
  }

  bool
  ParserImpl::meta() const
  {
    return meta_;
  }

  void
  ParserImpl::meta(bool b)
  {
    meta_ = b;
  }

  void
  ParserImpl::parse_(std::istream& source, const location_type* l)
  {
    TIMER_PUSH("parse");
    // Set up result_.

    // FIXME: This check will evaluate (void)*result_ in NDEBUG,
    // entailing an abortion since result_ == 0. Passert should
    // probably be fixed.
    // passert(*result_, !result_.get());

    result_.reset(new ParseResult);

    // Set up scanner.
    yyFlexLexer scanner;
    scanner.switch_streams(&source, 0);

    // Set up parser.
    parser_type p(*this, scanner);
#if defined YYDEBUG && YYDEBUG
    p.set_debug_level(debug_);
#endif

    // Save the current location so that we can restore it afterwards
    // (when reading the input flow, we want to be able to restore the
    // cursor after having handled a load command).
    libport::Finally finally;
    if (l)
      finally << libport::scoped_set(loc_, *l);

    // Parse.
    if (debug_)
      LIBPORT_ECHO(loc_ << "====================== Parse begin");

    result_->status = p.parse();
    if (debug_)
      LIBPORT_ECHO("====================== Parse end:" << std::endl
                   << *result_);

    TIMER_POP("parse");
  }

  parse_result_type
  ParserImpl::parse(const std::string& s, const location_type* l)
  {
    if (debug_)
      LIBPORT_ECHO("Parsing: " << s);
    std::istringstream is(s);
    parse_(is, l);
    if (debug_)
      LIBPORT_ECHO("Result: " << *result_);
    return result_;
  }

  parse_result_type
  ParserImpl::parse_file(const std::string& fn)
  {
    if (debug_)
      LIBPORT_ECHO("Parsing file: " << fn);
    std::ifstream f(fn.c_str());
    if (!f.good())
    {
      // Return an error instead of creating a valid empty ast.
      result_.reset(new ParseResult);
      result_->status = 1;
    }
    else
    {
      // FIXME: Leaks.
      location_type loc(new libport::Symbol(fn));
      parse_(f, &loc);
    }
    if (debug_)
      LIBPORT_ECHO("Result: " << *result_);
    return result_;
  }

  void
  ParserImpl::error(const location_type& l, const std::string& msg)
  {
    // We display "syntax error" ourselves.
    std::string err = msg;
    const char* synerr = "syntax error, ";
    if (boost::algorithm::starts_with(err, synerr))
      boost::algorithm::erase_head(err, strlen(synerr));
    result_->error(l, err);
  }

  void
  ParserImpl::warn(const location_type& l, const std::string& msg)
  {
    result_->warn(l, msg);
  }

  void
  ParserImpl::deprecated(const location_type& loc,
                         const std::string& what,
                         const std::string& suggestion)
  {
    // See Kernel1.deprecated.
    warn(loc,
         suggestion.empty()
         ? libport::format("`%s' is deprecated", what)
         : libport::format("`%s' is deprecated, use `%s'",
                           what, suggestion));
  }

}
