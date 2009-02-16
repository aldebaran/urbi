// Microsoft compiler does not allow main to be in a library.
// So we define one here.

#include <urbi/umain.hh>
#include <libport/detect-win32.h>

#ifndef WIN32
 __attribute__((visibility("default")))
#endif
int
main(int argc, const char* argv[])
{
  return urbi::main(argc, argv, true, true);
}

