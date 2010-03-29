/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cassert>
#include <urbi/kernel-version.hh>
#include <urbi/uabstractclient.hh>
#include <urbi/uclient.hh>

namespace urbi
{
  /*-----------------.
  | Kernel version.  |
  `-----------------*/

  int kernelMajor()
  {
    int res = getDefaultClient()->kernelMajor();
    passert(res, res != -1);
    return res;
  }

  int kernelMinor()
  {
    int res = getDefaultClient()->kernelMinor();
    passert(res, res != -1);
    return res;
  }

  const std::string& kernelVersion()
  {
    return getDefaultClient()->kernelVersion();
  }

}
