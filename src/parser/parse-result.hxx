#ifndef PARSER_PARSE_RESULT_HXX
# define PARSER_PARSE_RESULT_HXX

# include <parser/parse-result.hh>

namespace parser
{

  inline
  void
  ParseResult::error(const ast::loc& l, const std::string& msg)
  {
    errors_.error(l, msg);
  }

  inline
  void
  ParseResult::warn(const ast::loc& l, const std::string& msg)
  {
    errors_.warn(l, msg);
  }

  inline
  std::ostream&
  operator<< (std::ostream& o, const ParseResult& p)
  {
    return p.dump(o);
  }

}

#endif // !PARSER_PARSE_RESULT_HXX
