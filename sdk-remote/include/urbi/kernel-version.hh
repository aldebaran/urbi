/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/kernel-version.hh

#ifndef URBI_KERNEL_VERSION_HH
# define URBI_KERNEL_VERSION_HH

# include <libport/cstring>
# include <urbi/export.hh>

namespace urbi
{
  URBI_SDK_API int kernelMajor();
  URBI_SDK_API int kernelMinor();
  URBI_SDK_API const std::string& kernelVersion();
}

#endif // !URBI_KERNEL_VERSION_HH
