/// \file libuco/urbi-main.cc

#include <libport/cli.hh>
#include <libport/containers.hh>
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

namespace urbi
{

  int
  main(int argc, const char* argv[], bool block)
  {
    libport::cli_args_type args;
    // For some reason, I failed to use std::copy here.
    for (int i = 0; i < argc; ++i)
      args << std::string(argv[i]);
    return main(args, block);
  }

}
