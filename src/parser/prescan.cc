/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <sstream>

#include <libport/echo.hh>
#include <libport/buffer-stream.hh>
#include <parser/prescan.hh>
#include <parser/utoken.hh>

namespace parser
{

  long
  prescan(const char* buf, size_t len)
  {
    yyFlexLexer scanner;
    // Avoid unnecessary copy.
    libport::BufferStream is(buf, len);
    scanner.switch_streams(&is, 0);

    // In prescanner mode, return 1 if complete, -1 on EOF, 0 if incomplete.
    yy::parser::symbol_type s = scanner.yylex();
    yy::parser::token_type t = s.token();
    aver(t == yy::parser::token::TOK_PRE_EOF
           || t == yy::parser::token::TOK_PRE_WANTS_MORE
           || t == yy::parser::token::TOK_PRE_COMPLETE);
    // The number of read bytes.
    size_t length = scanner.pre_length;

    LIBPORT_DEBUG("res: " << t << ", length: " << length);
    if (t == yy::parser::token::TOK_PRE_COMPLETE)
      return length;
    else
    {
      // We met EOF before reaching a terminator.
      return scanner.length_hint * (-1);
    }
  }
}
