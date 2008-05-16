#ifndef PARSER_UPARSER_HXX
# define PARSER_UPARSER_HXX

# include "parser/uparser.hh"

namespace parser
{

  inline
  UParser::ast_type*
  UParser::ast_get ()
  {
    return errors_.empty () ? ast_ : 0;
  }

  inline
  void
  UParser::ast_set (UParser::ast_type* ast)
  {
    if (ast)
      passert(ast_, !ast_);
    ast_ = ast;
  }

}

#endif // !PARSER_UPARSER_HXX
