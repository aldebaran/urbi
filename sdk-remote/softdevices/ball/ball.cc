/*! \file ball.cc
 *******************************************************************************
  
 File: ball.cc\n
 Implementation of the Ball class.
  
 This file if part of the 'ball' soft device\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */
 
#include "ball.h"

/*************************************/
// 'Ball' soft device initialization

   UStart(ball);

/*************************************/

void toto() {};
int toto2(UVar &v) {
    cout << "toto2 call with " << v.get_name() << endl;
};


// **************************************************************************	
//! Ball constructor.
ball::ball() :
  UObject("ball")
{ 
  UVarInit      (ball,x);
  UVarInit      (ball,y);
  UFunctionInit (ball,myfun);
  UFunctionInit (ball,myfun1);
  UEventInit    (ball,myevent);
   
  UMonitor(x);
  UMonitor(x, &ball::stuff);
  UMonitor(y, &ball::stuff2);
  UMonitor(x, &toto2);
  
  //UMonitor("camera.val",&toto2);
  x = 42;
}


//! Ball destructor.
ball::~ball()
{  
}

string
ball::myfun   (int n, string s)
{  
  return("hello!");
}

int
ball::myfun1   (double n)
{
  return ((int)(n + 1));
}
  
void
ball::myevent ()
{
}
