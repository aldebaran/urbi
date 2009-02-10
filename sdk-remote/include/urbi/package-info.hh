#ifndef URBI_PACKAGE_INFO_HH
# define URBI_PACKAGE_INFO_HH

# include <libport/fwd.hh>
# include <urbi/export.hh>

namespace urbi
{

  /// Package information about liburbi and UObjects.
  URBI_SDK_API const libport::PackageInfo& package_info();

}

#endif // !URBI_PACKAGE_INFO_HH
