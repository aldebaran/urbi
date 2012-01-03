/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
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

GD_CATEGORY(Urbi.Parser);

namespace parser
{

  /*-------------.
  | ParserImpl.  |
  `-------------*/

  static bool yydebug = getenv("URBI_PARSER");

  ParserImpl::ParserImpl(std::istream& input)
    : input_(input)
    , loc_()
    , synclines_()
    , result_(0)
    , debug_(yydebug)
    , meta_(false)
    , factory_(new ast::Factory)
    , initial_token_()
  {
    scanner_.switch_streams(&input, 0);
    scanner_.parser_impl_ = this;
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

    // Set up parser.
    parser_type p(*this);
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
    try
    {
      p.parse();
    }
    catch (const std::exception& e)
    {
      // We may come back on the very same input (for instance when in
      // interactive session, and hit C-c).  So clear the input, and
      // reinitialize the Flex scanner, otherwise it will be jammed at
      // the next run.
      input_.clear();
      scanner_.yyrestart(&input_);
      throw;
    }


    // Do not keep the result in the parser.
    parse_result_type res = result_;
    result_ = 0;

    TIMER_POP("parse");
    if (debug_ && res)
      GD_SINFO_DEBUG("Parse end:" << *res);

    if (!errors_.empty())
      throw errors_;

    return res;
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
