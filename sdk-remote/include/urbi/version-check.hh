/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_VERSION_CHECK_HH
# define URBI_VERSION_CHECK_HH

# include <string>
# include <libport/fwd.hh>
# include <urbi/export.hh>

namespace urbi
{

  /// Check Urbi SDK versions (compiling vs. loading), throw if mismatch.
  /// \param where        a string to name the module
  URBI_SDK_API
  int
  check_sdk_version(const std::string& where,
                    const libport::PackageInfo& compiler);

  class URBI_SDK_API VersionChecker
  {
  public:
    VersionChecker(const std::string& where,
                   const libport::PackageInfo& compiler);
  };
}

# if defined URBI_INHIBIT_REVISION_CHECK

#  define URBI_CHECK_SDK_VERSION(Where)
#  define URBI_CHECK_SDK_VERSION_BARE(Where)

# else // ! URBI_INHIBIT_REVISION_CHECK

#  include <libport/debug.hh>
#  include <libport/package-info.hh>
#  include <urbi/package-info.hh>
#  include <urbi/revision-stub.hh>

namespace urbi
{

  namespace
  {
    /// Check SDK version, only once per module.
    ATTRIBUTE_USED
    void
    check_sdk_version_once(const std::string& where,
                           const libport::PackageInfo& compiler)
    {
      static int i = check_sdk_version(where, compiler);
      LIBPORT_USE(i);
    }

    /// Information about the Urbi SDK that compiled this module.
    ATTRIBUTE_USED
    const libport::PackageInfo&
    compiler_info()
    {
      LIBPORT_PACKAGE_INFO_STATIC_VAR_(URBI_SDK_INFO_, pi);
      GD_CATEGORY(Urbi.Version);
      GD_FINFO_TRACE("compiler info: %s", pi);
      return pi;
    }
  }
}

# define URBI_CHECK_SDK_VERSION(Where)                          \
  ::urbi::check_sdk_version(Where, urbi::compiler_info())

/// Same as URBI_CHECK_SDK_VERSION, but callable from anywhere.
#  define URBI_CHECK_SDK_VERSION_BARE(Where)                    \
  static                                                        \
  ::urbi::VersionChecker                                        \
  LIBPORT_CAT(urbicheck, __LINE__) (Where,                      \
                                    urbi::compiler_info())

# endif // ! URBI_INHIBIT_REVISION_CHECK
#endif
