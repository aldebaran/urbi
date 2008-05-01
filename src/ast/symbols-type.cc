#include <libport/separator.hh>
#include <libport/symbol.hh>

#include "ast/symbols-type.hh"

namespace std
{
  std::ostream&
  operator<<(std::ostream& o, const ast::symbols_type& ss)
  {
    return o << libport::separate(ss, ", ");
  }
}
