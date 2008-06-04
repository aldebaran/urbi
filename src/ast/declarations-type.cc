#include <libport/indent.hh>
#include <libport/separator.hh>

#include <ast/declarations-type.hh>
#include <ast/print.hh>

namespace ast
{
  std::ostream&
  operator<<(std::ostream& o, const declarations_type& decs)
  {
    // Specifying template parameters is needed for gcc-3.
    bool tail = false;
    foreach (ast::rConstDeclaration dec, decs)
    {
      if (tail++)
        o << ", ";
      o << dec->what_get();
    }
    return o;
  }
}
