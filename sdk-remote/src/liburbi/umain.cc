/*! \file main.cc
 *******************************************************************************

 File: main.cc\n
 Constains the main() function.

 This file is part of liburbi-cpp\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <urbi/uobject.hh>
#ifdef WIN32
# include <windows.h>
# define usleep(a) Sleep(a/1000)
#else
# include <unistd.h>
#endif

namespace urbi
{
  int main(int argc, const char * argv[]);
};

int
main(int argc, char *argv[])
{
  urbi::main(argc, argv);
  while (1)
    usleep(100000); //urbi::execute();
};
