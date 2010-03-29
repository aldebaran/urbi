/*
 * Copyright (C) 2006-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file parser/uparser.cc

#include <parser/uparser.hh>
#include <parser/parser-impl.hh>

namespace parser
{

  /*----------.
  | UParser.  |
  `----------*/

  UParser::UParser()
    : pimpl_(new ParserImpl)
  {
  }

  UParser::UParser(const UParser& rhs)
    : pimpl_(new ParserImpl(*rhs.pimpl_))
  {
  }

  UParser::~UParser()
  {
  }

  void
  UParser::meta(bool b)
  {
    pimpl_->meta(b);
  }

  parse_result_type
  UParser::parse(const std::string& command,
                 const yy::location* loc)
  {
    return pimpl_->parse(command, loc);
  }

  parse_result_type
  UParser::parse_file(const std::string& fn)
  {
    return pimpl_->parse_file(fn);
  }

}
