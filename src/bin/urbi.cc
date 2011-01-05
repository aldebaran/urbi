/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <config.h>

#include <string>
#include <vector>

#include <libport/debug.hh>

#include <urbi/urbi-root.hh>

int
main(int argc, char* argv[])
{
  UrbiRoot urbi_root(argv[0],
# ifdef STATIC_BUILD
    true
# else
    false
# endif
    );

# ifndef STATIC_BUILD
  urbi_root.load_plugin();
# endif

  std::vector<std::string> args;
  for (int i=0; i< argc; ++i)
    args.push_back(argv[i]);
  return urbi_root.urbi_main(args, true, true);
}
