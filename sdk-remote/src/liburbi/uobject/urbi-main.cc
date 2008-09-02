/*! \file urbi-main.cc.cc
 *******************************************************************************

 File: urbi-main.cc\n
 Implementation of the urbi_main function.

 This file is part of LIBURBI\n
 Copyright (c) 2004, 2005, 2006, 2007, 2008 Gostai S.A.S.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <urbi/uobject.hh>

extern "C"
{
  int urbi_main(int argc, const char* argv[], int block)
  {
    return urbi::main(argc, argv, block);
  }
}
