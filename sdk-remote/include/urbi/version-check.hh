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
#  include <libport/debug.hh>

# ifndef URBI_INHIBIT_REVISION_CHECK
#  include <libport/package-info.hh>
#  include <urbi/package-info.hh>
#  include <urbi/revision.hh>
# endif

namespace urbi
{
  /// Check SDK version, throw if mismatch.
  inline int check_sdk_version(std::string where = "")
  {
    LIBPORT_USE(where);
# ifndef URBI_INHIBIT_REVISION_CHECK
    GD_CATEGORY(Urbi);
    const libport::PackageInfo& info = urbi::package_info();
    if (!where.empty())
      where += ": ";
    std::string version_eff = info.get("version");
    std::string revision_eff = info.get("revision");
    std::string version_exp = URBI_SDK_VERSION;
    std::string revision_exp = URBI_SDK_REVISION;
    GD_FINFO_TRACE("%sCompiled version %s, loaded version %s",
                   where, revision_exp, revision_eff);
    if (revision_eff != revision_exp)
    {
      std::string expected = version_exp;
      std::string effective  = version_eff;
      if (expected == effective)
      {
        expected += " " + revision_exp;
        effective += " " + revision_eff;
      }
      std::string msg(libport::format
                      ("%sCompiled with Urbi SDK version %s,"
                       " but is loaded in version %s",
                       where, expected, effective));

      const char* varname = "URBI_ACCEPT_BINARY_MISMATCH";
      const bool fatal = !libport::getenv(varname);
      if (fatal)
      {
        GD_ERROR(msg);
        GD_FERROR("define %s to bypass this error", varname);
        throw std::runtime_error(msg);
      }
      else
        GD_WARN(msg);
    }
#endif
    return 0;
  }

  /// Check sdk version, only once per module.
  inline void check_sdk_version_once(const std::string& where = "")
  {
    static int i = check_sdk_version(where);
    LIBPORT_USE(i);
  }

  class VersionChecker
  {
  public:
    VersionChecker(const std::string& where = "")
    {
      GD_CATEGORY(Urbi);
      GD_SINFO_DUMP("VersionChecker instanciating...");
      check_sdk_version(where);
    }
  };
}

#define URBI_CHECK_SDK_VERSION(Where)           \
  ::urbi::check_sdk_version(Where)

/// Same as URBI_CHECK_SDK_VERSION, but callable from anywhere.
#define URBI_CHECK_SDK_VERSION_BARE(Where)                      \
  ::urbi::VersionChecker LIBPORT_CAT(urbicheck, __LINE__)       \
  = ::urbi::VersionChecker(Where)

#endif
