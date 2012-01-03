/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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
