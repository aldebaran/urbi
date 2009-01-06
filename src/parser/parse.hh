/// \file parser/parse.hh

#ifndef PARSER_PARSE_HH
# define PARSER_PARSE_HH

# include <memory>

# include <parser/fwd.hh>
# include <parser/location.hh>
# include <parser/parse-result.hh>

namespace parser
{

  /// Parse \a cmd, and return an object that holds the result and the
  /// errors.
  /// Parse \a cmd, and return an object that holds the result and the
  /// errors.  Meta variables are activated.
  parse_result_type parse_meta(const std::string& cmd, const yy::location& loc);
  parse_result_type parse(const std::string& cmd,
                          const yy::location& loc = yy::location());

  /// Parse a file.
  /// \see parse().
  parse_result_type parse_file(const std::string& file);

}

#endif // !PARSER_PARSE_HH
