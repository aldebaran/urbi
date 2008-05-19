/// \file parser/uparser.cc

#include "parser/uparser.hh"
#include "parser/parser-impl.hh"

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

  parse_result_type
  UParser::parse(const std::string& command)
  {
    return pimpl_->parse(command);
  }

  parse_result_type
  UParser::parse(Tweast& t)
  {
    return pimpl_->parse(t);
  }

  parse_result_type
  UParser::parse_file(const std::string& fn)
  {
    return pimpl_->parse_file(fn);
  }

}
