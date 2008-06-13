#include <iostream>
#include <libport/indent.hh>

#include <kernel/debug.hh>

namespace
{

  static int init_cerr_indent ()
  {
    std::cerr << libport::resetindent;
    return 42;
  }
  static int init = init_cerr_indent();

}
