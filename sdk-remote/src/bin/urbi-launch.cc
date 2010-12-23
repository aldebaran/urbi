/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// This file *must* remain with absolutely no run-time dependency.

#include <urbi/urbi-root.hh>

#include <libport/config.h>

int main(int argc, char* argv[])
{
  UrbiRoot urbi_root(argv[0],
#ifdef STATIC_BUILD
                     true
#else
                     false
#endif
                     );
  return urbi_root.urbi_launch(argc, argv);
}
