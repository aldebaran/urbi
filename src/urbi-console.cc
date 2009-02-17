// Microsoft compiler does not allow main to be in a library.
// So we define one here.

#include <urbi/umain.hh>
#include <libport/detect-win32.h>
#include <libport/program-name.hh>

#ifndef WIN32
 __attribute__((visibility("default")))
#endif
int
main(int argc, char* argv[])
{
  libport::program_initialize(argc, argv);
  return urbi::main(argc, const_cast<const char**>(argv), true, true);
}

