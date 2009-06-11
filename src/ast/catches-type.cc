#include <ast/catches-type.hh>
#include <ast/print.hh>
#include <libport/indent.hh>

namespace ast
{
  std::ostream&
  operator<<(std::ostream& o, const catches_type& c)
  {
    // Specifying template parameters is needed for gcc-3.
    return o << libport::separate<const catches_type,
                                  std::ostream&(*)(std::ostream&)>
                                 (c, libport::iendl);
  }

}
