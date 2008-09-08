// Microsoft compiler does not allow main to be in a library.
// So we define one here.

#include <urbi/uobject.hh>

int
main(int argc, const char* argv[])
{
  return urbi::main(argc, argv);
}

