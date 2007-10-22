#if ! defined PACKAGE_INFO_HH
# define PACKAGE_INFO_HH

# include <string>

struct PackageInfo
{
  PackageInfo();
  const std::string
    bug_report,
    date,
    id,
    name,
    revision,
    string,
    tarname,
    version,
    version_rev;
};

extern const PackageInfo package_info;

#endif // ! PACKAGE_INFO_HH
