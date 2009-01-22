#ifndef FLOWER_FLOW_HH
# define FLOWER_FLOW_HH

# include <ast/nary-fwd.hh>
# include <urbi/export.hh>

namespace flower
{
  ast::rNary URBI_SDK_API flow(ast::rConstAst ast);

} // namespace flower

#endif // FLOWER_FLOW_HH
