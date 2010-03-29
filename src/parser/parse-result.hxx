/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef PARSER_PARSE_RESULT_HXX
# define PARSER_PARSE_RESULT_HXX

# include <parser/parse-result.hh>

namespace parser
{

  inline
  void
  ParseResult::error(const ast::loc& l, const std::string& msg)
  {
    errors_->error(l, msg);
  }

  inline
  void
  ParseResult::warn(const ast::loc& l, const std::string& msg)
  {
    errors_->warn(l, msg);
  }

  inline
  std::ostream&
  operator<< (std::ostream& o, const ParseResult& p)
  {
    return p.dump(o);
  }

}

#endif // !PARSER_PARSE_RESULT_HXX
