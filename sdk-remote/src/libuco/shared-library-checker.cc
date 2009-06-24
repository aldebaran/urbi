#include <cstring>
#include <iostream>

#include <libport/cstdlib>
#include <libport/program-name.hh>
#include <libport/sysexits.hh>
#include <libport/unistd.h>

#include <urbi/uobject.hh>

// Check that we are not loading the libuobject several times.  It
// happens if for instance, by error, you link an uobject with
// libuobject, and then load it with urbi-launch, which dlopens one by
// itself.  In that case the symbols coexist.
//
// We use something which is guaranteed to be unique to ensure there
// is just one instance that is run: an envvar.
namespace urbi
{
  namespace libuobject
  {
    struct shared_library_checker
    {
      static
      std::string
      mode()
      {
        return isPluginMode() ? "plugin" : "remote";
      }

      shared_library_checker()
      {
        const char* var = "__URBI_LIBUOBJECT_MODE";
        if (const char* val = getenv(var))
          std::cerr
            << libport::program_name()
            << ": inconsistency detected: libuobject already loaded (as "
            << val << " mode)."
            << std::endl
            << libport::program_name()
            << ": maybe you loaded an UObject linked with libuobject?"
            << std::endl
            << libport::program_name()
            << ": UObjects must *not* be linked with libuobject."
            << std::endl
            << libport::exit(EX_CONFIG);
        else
          setenv(var, mode().c_str(), 1);
      }
    };

    // Static instance whose ctor will check the desired property.
    shared_library_checker checker;
  }

}
