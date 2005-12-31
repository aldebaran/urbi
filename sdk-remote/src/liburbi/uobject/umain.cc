/*! \file ballmain.cc
 *******************************************************************************

 File: ballmain.cc\n
 Constains the main() function.

 This file if part of the 'ball' soft device\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include "uobject.h"

int
main(int argc, char *argv[])
{
  URBI::URBIMain(argc, argv);
  urbi::execute();
};
