/// \file urbi/kernel-version.hh

#ifndef URBI_KERNEL_VERSION_HH
# define URBI_KERNEL_VERSION_HH

# include <cstring>
# include <urbi/export.hh>

namespace urbi
{
  URBI_SDK_API int kernelMajor();
  URBI_SDK_API int kernelMinor();
  URBI_SDK_API const std::string& kernelVersion();
}

#endif // !URBI_KERNEL_VERSION_HH
