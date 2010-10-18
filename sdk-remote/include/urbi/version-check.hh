/*
 * Copyright (C) 2010, Gostai S.A.S.
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
/** Check the compile/time and library SDK version, throws if mismatch.
 * Must be called from within a function.
 */
#  define URBI_CHECK_SDK_VERSION()                                      \
  do                                                                    \
  {                                                                     \
    GD_CATEGORY(Urbi);                                                  \
    const libport::PackageInfo& info = urbi::package_info();            \
    std::string version_eff = info.get("version");                      \
    std::string revision_eff = info.get("revision");                    \
    std::string version_exp = URBI_SDK_VERSION;                         \
    std::string revision_exp = URBI_SDK_REVISION;                       \
    GD_FINFO_TRACE("Compiled version %s, loaded version %s", revision_exp, \
                   revision_eff);                                       \
    if (revision_eff != revision_exp)                                   \
    {                                                                   \
      std::string expected = version_exp;                               \
      std::string effective  = version_eff;                             \
      if (expected == effective)                                        \
      {                                                                 \
        expected += " " + revision_exp;                                 \
        effective += " " + revision_eff;                                \
      }                                                                 \
      std::string msg(libport::format                                   \
                      ("Module was compiled with Urbi SDK version %s,"  \
                       " but is loaded in version %s",                  \
                       expected, effective));                           \
                                                                        \
      const std::string varname = "URBI_ACCEPT_BINARY_MISMATCH";        \
      const bool fatal = !libport::getenv(varname.c_str());             \
      if (fatal)                                                        \
      {                                                                 \
        GD_ERROR(msg);                                                  \
        GD_FERROR("define %s to bypass this error", varname);           \
        throw std::runtime_error(msg);                                  \
      }                                                                 \
      else                                                              \
        GD_WARN(msg);                                                   \
    }                                                                   \
  } while (0)                                                           \

# else
#  define URBI_CHECK_SDK_VERSION()
# endif
namespace urbi
{
  /// Check sdk version, throw if mismatch.
  inline int check_sdk_version()
  {
    URBI_CHECK_SDK_VERSION();
    return 0;
  }
  /// Check sdk version, only once per module.
  inline void check_sdk_version_once()
  {
    static int i = check_sdk_version();
    (void)i;
  }
  class VersionChecker
  {
  public:
    VersionChecker()
    {
      GD_CATEGORY(Urbi);
      GD_SINFO_DUMP("VersionChecker instanciating...");
      URBI_CHECK_SDK_VERSION();
    }
  };
}
/// Same as URBI_CHECK_SDK_VERSION, but callable from anywhere.
#define URBI_CHECK_SDK_VERSION_BARE \
::urbi::VersionChecker LIBPORT_CAT(urbicheck, __LINE__) \
  = ::urbi::VersionChecker()
#endif
