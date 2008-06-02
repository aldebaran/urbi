/**
 ** \file parser/fwd.hh
 ** \brief Forward declarations of all parser classes
 ** (needed by the visitors).
 */

#ifndef PARSER_FWD_HH
# define PARSER_FWD_HH

# include <memory>

// flex-lexer.hh.
class FlexLexer;

namespace parser
{
  // tweast.hh.
  class Tweast;

  // parser-impl.hh.
  class ParserImpl;

  // parse-result.hh.
  class ParseResult;

  // The actual definition.
  typedef std::auto_ptr<ParseResult> parse_result_type;

  // uparser.hh.
  class UParser;
}

namespace yy
{
  class location;
  class position;
}

#endif // !PARSER_FWD_HH
