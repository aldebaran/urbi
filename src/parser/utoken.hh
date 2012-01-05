/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef PARSER_UTOKEN_HH
# define PARSER_UTOKEN_HH

# include <parser/flex-lexer.hh>
# include <parser/ugrammar.hh>

# undef  YY_DECL
# define YY_DECL							\
  parser::ParserImpl::symbol_type yyFlexLexer::yylex()

#endif // !PARSER_UTOKEN_HH
