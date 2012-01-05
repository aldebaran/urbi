/*
 * Copyright (C) 2006-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file parser/uparser.cc

#include <fstream>

#include <parser/uparser.hh>
#include <parser/parser-impl.hh>

namespace parser
{

  /*----------.
  | UParser.  |
  `----------*/

  UParser::UParser(std::istream& input, const yy::location* loc)
    : pimpl_(0)
    , stream_(&input)
    , stream_delete_(false)
    , oneshot_(false)
    , loc_(loc)
  {
    pimpl_ = new ParserImpl(*stream_);
  }

  UParser::UParser(const std::string& code, const yy::location* loc)
    : pimpl_(0)
    , stream_(new std::stringstream(code))
    , stream_delete_(true)
    , oneshot_(true)
    , loc_(loc)
  {
    pimpl_ = new ParserImpl(*stream_);
  }

  UParser::UParser(const libport::path& file)
    : pimpl_(0)
    , stream_(0)
    , stream_delete_(true)
    , oneshot_(true)
    , loc_file_(new libport::Symbol(file.to_string())) // FIXME: leaks.
    , loc_(&loc_file_)
  {
    std::ifstream* input = new std::ifstream(file.to_string().c_str());
    if (!input->good())
      throw 42; // FIXME
    stream_ = input;
    pimpl_ = new ParserImpl(*stream_);
  }

  UParser::UParser(const UParser& rhs)
    : pimpl_(new ParserImpl(*rhs.pimpl_))
  {
  }

  UParser::~UParser()
  {
    if (stream_delete_)
      delete stream_;
    delete pimpl_;
  }

  void
  UParser::meta(bool b)
  {
    pimpl_->meta(b);
  }

  parse_result_type
  UParser::parse()
  {
    pimpl_->initial_token_set(oneshot_
                              ? ::yy::parser::token::TOK_MODE_EXPS
                              : ::yy::parser::token::TOK_MODE_EXP);
    return pimpl_->parse(loc_);
  }

  void
  UParser::oneshot_set(bool v)
  {
    oneshot_ = v;
  }
}
