#ifndef PARSER_UTOKEN_HH
# define PARSER_UTOKEN_HH

# include <parser/flex-lexer.hh>
# include <parser/ugrammar.hh>

# undef  YY_DECL
# define YY_DECL							\
  parser::ParserImpl::token_type                                        \
  yyFlexLexer::yylex(parser::ParserImpl::semantic_type* valp,		\
		     parser::ParserImpl::location_type* locp,		\
		     parser::ParserImpl* up)

#endif // !PARSER_UTOKEN_HH
