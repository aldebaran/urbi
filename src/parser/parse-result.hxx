#ifndef PARSER_PARSE_RESULT_HXX
# define PARSER_PARSE_RESULT_HXX

# include <parser/parse-result.hh>

namespace parser
{

  inline
  void
  ParseResult::error (const std::string& msg)
  {
    reported_ = false;
    errors_.push_back(msg);
  }

  inline
  void
  ParseResult::warn (const std::string& msg)
  {
    reported_ = false;
    warnings_.push_back(msg);
  }

  inline
  std::ostream&
  operator<< (std::ostream& o, const ParseResult& p)
  {
    return p.dump(o);
  }

}

#endif // !PARSER_PARSE_RESULT_HXX
