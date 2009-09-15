/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/program-name.hh>

extern "C"
{
  inline
  int urbi_main(int argc, const char* argv[],
                bool block, bool errors)
  {
    return urbi::main(argc, argv, block, errors);
  }

  inline
  int urbi_main_args(const libport::cli_args_type& args,
                     bool block, bool errors)
  {
    return urbi::main(args, block, errors);
  }
}

namespace urbi
{
  inline
  int
  main(int argc, const char* argv[], bool block, bool errors)
  {
    return main(libport::cli_args_type(argv, argv + argc), block, errors);
  }

}
