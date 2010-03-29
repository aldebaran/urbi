/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cerrno>
#include <fstream>
#include <kernel/config.h>
#include <kernel/lock.hh>
#include <kernel/urbi-sdk-key.hh>
#include <kernel/userver.hh>
#include <libport/sysexits.hh>
#include <urbi/exit.hh>

namespace kernel
{
  ATTRIBUTE_NORETURN
  static inline void server_error(const std::string& msg);

  static inline
  void
  server_error(const std::string& msg)
  {
    throw urbi::Exit(EX_OSFILE, msg);
  }

#define FAIL(Format, ...)                               \
  server_error(libport::format(Format), ## __VA_ARGS__)

  /// Check that this server is allowed to run.  Die if not.
  void
  lock_check(const UServer& s)
  {
    (void) s;
# ifdef URBI_SDK_KEY
    // Look for the key file.
# define URBI_SDK_KEY_FILE "urbi-sdk.key"
    std::string path;
    try
    {
      path = s.find_file(URBI_SDK_KEY_FILE);
    }
    catch (const libport::file_library::Not_found&)
    {
      FAIL("cannot find Urbi SDK key file `%s': %s",
           URBI_SDK_KEY_FILE, strerror(errno));
    }

    // Load it.
    std::string key;
    try
    {
      key = libport::read_file(path);
    }
    catch (const std::exception& e)
    {
      FAIL("cannot read Urbi SDK key file `%s': %s",
           path, e.what());
    }

    // Play a bit with the key itself, so that it is not too easy,
    // using strings(1), to find the key in the object file.
    foreach (char& c, key)
      c ^= 23;

    // Check that it matches with our own key.
    if (key != URBI_SDK_KEY)
      FAIL("invalid Urbi SDK key: %s", path);
# endif // !URBI_SDK_KEY
  }

}
