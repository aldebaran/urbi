#ifndef URBI_PACKAGE_INFO_HH
# define URBI_PACKAGE_INFO_HH

# include <libport/package-info.hh>
# include <urbi/export.hh>

namespace urbi
{

  /// Package information about liburbi and UObjects.
  USDK_API const libport::PackageInfo& package_info();

}

#endif // !URBI_PACKAGE_INFO_HH
