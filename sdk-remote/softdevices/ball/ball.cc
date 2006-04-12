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
    echo("toto2 call with %s\n",v.get_name().c_str());
};


// **************************************************************************	
//! Ball constructor.
ball::ball(string s) :
  UObject(s)
{ 
  UBindVar      (ball,x);
  UBindVar      (ball,y);
  UBindVar      (ball,truc);
  UBindFunction (ball,init);
  UBindFunction (ball,myfun);
  UBindFunction (ball,myfun1);
  UBindEvent    (ball,myevent);   
  UBindEventEnd (ball,myevent,endevent);

  USync(x);
  USync(truc);
  UNotifyChange(x, &ball::stuff);
  UNotifyChange(y, &ball::stuff2);
  UNotifyChange(x, &toto2);
  
  //UNotifyChange("camera.val",&toto2);
  x = 42;
}


//! Ball destructor.
ball::~ball()
{  
}

string
ball::myfun   (int n, string s)
{  
  echo("I'm in %s.myfun(%d,%s)\n",name.c_str(),n,s.c_str());
  
  return("hello!");
}

int
ball::myfun1   (double n)
{ 
  echo("I'm in %s.myfun1(%lf)\n",name.c_str(),n);
  return ((int)(n + 1));
}
  
void
ball::myevent (int n)
{
  echo("Event '%s.myevent' received with param %d\n",name.c_str(),n);
}

void
ball::endevent ()
{
  echo("Event '%s.myevent' ended \n",name.c_str());
}
