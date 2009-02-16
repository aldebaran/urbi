/// \file libuco/urbi-main.cc

#include <libport/cli.hh>
#include <libport/containers.hh>
#include <urbi/umain.hh>

extern "C"
{
  int urbi_main(int argc, const char* argv[],
                bool block, bool errors)
  {
    return urbi::main(argc, argv, block, errors);
  }

  int urbi_main_args(const libport::cli_args_type& args,
                     bool block, bool errors)
  {
    return urbi::main(args, block, errors);
  }
}

namespace urbi
{

  int
  main(int argc, const char* argv[], bool block, bool errors)
  {
    libport::cli_args_type args;
    // For some reason, I failed to use std::copy here.
    for (int i = 0; i < argc; ++i)
      args << std::string(argv[i]);
    return main(args, block, errors);
  }

}
