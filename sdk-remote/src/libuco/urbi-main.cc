/// \file libuco/urbi-main.cc

#include <urbi/umain.hh>

extern "C"
{
  int urbi_main(int argc, const char* argv[], int block)
  {
    return urbi::main(argc, argv, block);
  }
}
