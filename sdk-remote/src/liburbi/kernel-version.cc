#include <libport/assert.hh>
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
