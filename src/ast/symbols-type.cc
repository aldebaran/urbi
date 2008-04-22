#include <libport/foreach.hh>
#include <libport/symbol.hh>

#include "ast/symbols-type.hh"

namespace std
{
  std::ostream&
  operator<<(std::ostream& o, const ast::symbols_type& ss)
  {
    // I didn't manage to use libport::separator here.
    bool tail = false;
    foreach (libport::Symbol *s, ss)
    {
      if (tail++)
	o << ", ";
      o << *s;
    }
    return o;
  }
}
