#include <ast/dot-print.hh>
#include <ast/dot-printer.hh>

namespace ast
{
  void
  dot_print(rConstAst ast, std::ostream& stream)
  {
    DotPrinter p(stream);
    p(ast.get());
  }
}
