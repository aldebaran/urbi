#ifndef PARSER_UTOKEN_HH
# define PARSER_UTOKEN_HH

# include "parser/flex-lexer.hh"
# include "parser/ugrammar.hh"

# undef  YY_DECL
# define YY_DECL							\
  parser::UParser::token_type						\
  yyFlexLexer::yylex(parser::UParser::semantic_type* valp,		\
		     parser::UParser::location_type* locp,		\
		     parser::UParser* up)

#endif // !PARSER_UTOKEN_HH
