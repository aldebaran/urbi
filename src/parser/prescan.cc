#include <sstream>

#include "parser/prescan.hh"
#include "parser/utoken.hh"

namespace parser
{

  size_t
  prescan(const char* buf)
  {
    yyFlexLexer scanner;
    std::istringstream is(buf);
    scanner.switch_streams(&is, 0);

    // In prescanner mode, return 1 if complete, -1 on EOF, 0 if incomplete.
    int c = scanner.yylex();
    passert(c, c == pre_eof || c == pre_wants_more || c == pre_complete);
    // The number of read bytes.
    size_t length = scanner.pre_length;
    ECHO("res: " << c << ", length: " << length);
    if (c == pre_complete)
      return length;
    else
      // We met EOF before reaching a terminator.
      return 0;
  }
}
