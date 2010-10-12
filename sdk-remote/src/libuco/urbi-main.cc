/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuco/urbi-main.cc

#include <libport/cli.hh>
#include <libport/containers.hh>
#include <urbi/umain.hh>
#include <urbi/urbi-root.hh>

extern "C"
{
  int urbi_main(int argc, const char* argv[],
                UrbiRoot& urbi_root, bool block, bool errors)
  {
    libport::program_initialize(argc, argv);

    return urbi::main(argc, argv, urbi_root, block, errors);
  }

  int urbi_main_args(const libport::cli_args_type& args,
                     UrbiRoot& urbi_root, bool block, bool errors)
  {
    libport::program_initialize(args);

    return urbi::main(args, urbi_root, block, errors);
  }
}

namespace urbi
{

  int
  main(int argc, const char* argv[], UrbiRoot& urbi_root, bool block, bool errors)
  {
    libport::program_initialize(argc, argv);

    libport::cli_args_type args;
    // For some reason, I failed to use std::copy here.
    for (int i = 0; i < argc; ++i)
      args << std::string(argv[i]);
    return main(args, urbi_root, block, errors);
  }

}
