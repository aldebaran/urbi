/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file parser/fwd.hh
 ** \brief Forward declarations of all parser classes
 ** (needed by the visitors).
 */

#ifndef PARSER_FWD_HH
# define PARSER_FWD_HH

# include <ast/fwd.hh>
# include <memory>

// flex-lexer.hh.
class yyFlexLexer;

namespace parser
{
  // parser-impl.hh.
  class ParserImpl;

  // The actual definition.
  typedef ast::rExp parse_result_type;

  // uparser.hh.
  class UParser;
}

namespace yy
{
  class location;
  class position;
}

#endif // !PARSER_FWD_HH
