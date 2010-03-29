/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef KERNEL_UBANNER_HH
# define KERNEL_UBANNER_HH

# include <libport/fwd.hh>
# include <urbi/export.hh>

namespace kernel
{
  URBI_SDK_API std::ostream&
  userver_package_info_dump(std::ostream& o);
}
#endif // !KERNEL_UBANNER_HH
