#ifndef REWRITE_REWRITE_HH
# define REWRITE_REWRITE_HH

# include <ast/fwd.hh>

namespace rewrite
{
  ast::rNary rewrite(ast::rConstNary a);
  ast::rPipe pattern_bind(const ast::rConstAst& pattern,
                          const ast::rExp& exp,
                          bool before = true);
}

#endif
