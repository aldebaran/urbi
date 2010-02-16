/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_SDK_HXX
# define URBI_SDK_HXX

#include <urbi/object/global.hh>
#include <urbi/sdk.hh>

namespace urbi
{
  rObject global()
  {
    return object::global_class;
  }
}

#endif
