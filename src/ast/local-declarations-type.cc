#include <libport/indent.hh>
#include <libport/separate.hh>

#include <ast/local-declarations-type.hh>
#include <ast/print.hh>

namespace ast
{
  std::ostream&
  operator<<(std::ostream& o, const local_declarations_type& decs)
  {
    bool tail = false;
    foreach (ast::rConstLocalDeclaration dec, decs)
    {
      if (tail++)
        o << ", ";
      o << dec->what_get();
    }
    return o;
  }
}
