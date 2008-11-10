/// \file libuco/urbi-main.cc

#include <urbi/umain.hh>

extern "C"
{
  int urbi_main(int argc, const char* argv[], bool block)
  {
    return urbi::main(argc, argv, block);
  }

  int urbi_main_args(const libport::cli_args_type& args, bool block)
  {
    return urbi::main(args, block);
  }
}
