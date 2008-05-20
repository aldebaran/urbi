#include "ast/pretty-printer.hh"
#include "ast/print.hh"

namespace ast
{

  std::ostream&
  operator<< (std::ostream& o, const Ast& a)
  {
    PrettyPrinter p (o);
    p(a);
    return o;
  }

} // namespace ast


