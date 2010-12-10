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

  ParserImpl::ParserImpl(std::istream& input)
    : loc_()
    , synclines_()
    , result_(0)
    , debug_(yydebug)
    , meta_(false)
    , factory_(new ast::Factory)
    , initial_token_()
  {
    scanner_.switch_streams(&input, 0);
  }

  void
  ParserImpl::initial_token_set(token_type initial_token)
  {
    initial_token_ = initial_token;
  }

  boost::optional<ParserImpl::token_type>&
  ParserImpl::initial_token_get()
  {
    return initial_token_;
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

  parse_result_type
  ParserImpl::parse(const location_type* l)
  {
    TIMER_PUSH("parse");

    // Clear previous errors.
    errors_.clear();

    // Set up result_.

    // FIXME: This check will evaluate (void)*result_ in NDEBUG,
    // entailing an abortion since result_ == 0. Passert should
    // probably be fixed.
    // passert(*result_, !result_.get());
    result_.reset(new ParseResult);

    // Set up parser.
    parser_type p(*this, scanner_);
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
    p.parse();
    if (debug_)
      LIBPORT_ECHO("====================== Parse end:" << std::endl
                   << *result_);

    TIMER_POP("parse");
    if (debug_)
      LIBPORT_ECHO("Result: " << *result_);

    if (!errors_.empty())
      throw errors_;

    return result_;
  }

  void
  ParserImpl::error(const location_type& l, const std::string& msg)
  {
    std::string err = msg;
    const char* synerr = "syntax error, ";
    if (boost::algorithm::starts_with(err, synerr))
      boost::algorithm::erase_head(err, strlen(synerr));
    errors_.err(l, err, "syntax error");
  }

  void
  ParserImpl::warn(const location_type& l, const std::string& msg)
  {
    errors_.warn(l, msg);
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
