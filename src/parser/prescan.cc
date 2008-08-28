#include <sstream>

#include <parser/prescan.hh>
#include <parser/utoken.hh>

namespace parser
{

  size_t
  prescan(const char* buf)
  {
    yyFlexLexer scanner;
    std::istringstream is(buf);
    scanner.switch_streams(&is, 0);

    // In prescanner mode, return 1 if complete, -1 on EOF, 0 if incomplete.
    yy::parser::symbol_type s = scanner.yylex();
    yy::parser::token_type t = s.token();
    assert(t == yy::parser::token::TOK_PRE_EOF
           || t == yy::parser::token::TOK_PRE_WANTS_MORE
           || t == yy::parser::token::TOK_PRE_COMPLETE);
    // The number of read bytes.
    size_t length = scanner.pre_length;
    ECHO("res: " << t << ", length: " << length);
    if (t == yy::parser::token::TOK_PRE_COMPLETE)
      return length;
    else
      // We met EOF before reaching a terminator.
      return 0;
  }
}
