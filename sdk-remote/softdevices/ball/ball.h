/*! \file ball.h
 *******************************************************************************

 File: ball.h\n
 Definition of the ball class.

 This file is part of the 'ball' soft device\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include "uobject.h"

//! ball class definition
class ball : public UObject
{
 public:
  
  ball();
  ~ball();  

  UVar x;
  UVar y;

  string myfun   (int,string);
  int    myfun1  (double);
  void   myevent ();

  int stuff() {};
  int stuff2(UVar &v) {};    
};

