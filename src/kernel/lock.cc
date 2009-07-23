#include <cerrno>
#include <fstream>
#include <kernel/config.h>
#include <kernel/lock.hh>
#include <kernel/urbi-sdk-key.hh>
#include <kernel/userver.hh>
#include <libport/sysexits.hh>
#include <urbi/exit.hh>

namespace kernel
{
  static inline void server_error(const std::string& msg) ATTRIBUTE_NORETURN;

  static inline
  void
  server_error(const std::string& msg)
  {
    throw urbi::Exit(EX_OSFILE, msg);
  }

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
      server_error(std::string()
                   + "cannot find Urbi SDK key file " URBI_SDK_KEY_FILE ": "
                   + strerror(errno));
    }

    // Load it.
    std::string key;
    {
      std::ifstream is(path.c_str(), std::ios::binary);
      if (!is)
        server_error("cannot open Urbi SDK key file " + path
                     + ": " + strerror(errno));
      while (is.good())
      {
        static char buf[BUFSIZ];
        is.read(buf, sizeof buf);
        key.append(buf, is.gcount());
      }
      if (is.bad())
        server_error("cannot load Urbi SDK key file " + path
                     + ": " + strerror(errno));
    }

    // Play a bit with the key itself, so that it is not too easy,
    // using strings(1), to find the key in the object file.
    foreach (char& c, key)
      c ^= 23;

    // Check that it matches with our own key.
    if (key != URBI_SDK_KEY)
      server_error("invalid Urbi SDK key: " + path);
# endif // !URBI_SDK_KEY
  }

}
