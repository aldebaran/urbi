/// \file parser/parse.hh

#ifndef PARSER_PARSE_HH
# define PARSER_PARSE_HH

# include <memory>

# include "parser/fwd.hh"

namespace parser
{

  /// Parse \a cmd, and return an object that holds the result and the
  /// errors.
  parse_result_type parse(const std::string& cmd);

  /// Parse a Tweast and return the (desugared) AST.
  parse_result_type parse(Tweast& t);

  /// Parse a file.
  /// \see parse().
  parse_result_type parse_file(const std::string& file);

}

#endif // !PARSER_PARSE_HH
