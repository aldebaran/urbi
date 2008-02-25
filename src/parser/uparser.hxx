#ifndef PARSER_UPARSER_HXX
# define PARSER_UPARSER_HXX

# include "parser/uparser.hh"

inline
UParser::ast_type*
UParser::ast_get ()
{
  return errors_.empty () ? ast_ : 0;
}

inline
std::auto_ptr<UParser::ast_type>
UParser::ast_take ()
{
  ast_type* res = ast_get();
  assert(res);
  ast_set(0);
  return std::auto_ptr<UParser::ast_type>(res);
}

inline
void
UParser::ast_set (UParser::ast_type* ast)
{
  if (ast)
    passert(ast_, !ast_);
  ast_ = ast;
}

#endif // !PARSER_UPARSER_HXX
