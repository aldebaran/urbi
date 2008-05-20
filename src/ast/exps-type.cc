#include <libport/indent.hh>
#include <libport/separator.hh>

#include "ast/exp.hh"
#include "ast/exps-type.hh"
#include "ast/print.hh"

namespace ast
{
  std::ostream&
  operator<<(std::ostream& o, const exps_type& es)
  {
    // Specifying template parameters is needed for gcc-3.
    return o << libport::separate<const exps_type,
                                  std::ostream&(*)(std::ostream&)>
                                 (es, libport::iendl);
  }
}
