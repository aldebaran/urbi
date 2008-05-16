#ifndef PARSER_PARSER_IMPL_HXX
# define PARSER_PARSER_IMPL_HXX

# include "parser/uparser.hh"

namespace parser
{

  inline
  ParserImpl::ast_type*
  ParserImpl::ast_get ()
  {
    return errors_.empty () ? ast_ : 0;
  }

  inline
  void
  ParserImpl::ast_set (ParserImpl::ast_type* ast)
  {
    if (ast)
      passert(ast_, !ast_);
    ast_ = ast;
  }

}

#endif // !PARSER_PARSER_IMPL_HXX
