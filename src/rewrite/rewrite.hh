#ifndef REWRITE_REWRITE_HH
# define REWRITE_REWRITE_HH

# include <ast/fwd.hh>

namespace rewrite
{
  ast::rNary rewrite(ast::rConstNary a);
  ast::rExp rewrite(ast::rConstExp a);
}

#endif
