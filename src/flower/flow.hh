/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef FLOWER_FLOW_HH
# define FLOWER_FLOW_HH

# include <ast/exp.hh>
# include <urbi/export.hh>

namespace flower
{
  template <typename T>
  URBI_SDK_API libport::intrusive_ptr<T>
  flow(libport::intrusive_ptr<const T> ast);
} // namespace flower

#endif // FLOWER_FLOW_HH
