/*! \file ucommandqueue.cc
 *******************************************************************************

 File: ucommandqueue.cc\n
 Implementation of the UCommandQueue class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <cstdlib>
#include "libport/cstring"

#include "uqueue.hh"
#include "ucommandqueue.hh"
#include "userver.hh"

//! UCommandQueue constructor.
/*! UCommandQueue constructor simply calls the UQueue constructor with the same
    behavior.
*/
UCommandQueue::UCommandQueue  (int minBufferSize,
				int maxBufferSize,
				int adaptive) :
  UQueue (minBufferSize,
	  maxBufferSize,
	  adaptive),
  cursor_         (0),
  bracketlevel_   (0),
  sbracketlevel_  (0),
  parenlevel_     (0),
  discard_        (false),
  closechar_      (' '),
  closechar2_     (' ')
{
  ADDOBJ(UCommandQueue);
  FREEOBJ(UQueue); // A tester...
}

//! UCommandQueue destructor.
UCommandQueue::~UCommandQueue()
{
  ADDOBJ(UQueue);
  FREEOBJ(UCommandQueue);
}


//! Pops the next command in the queue.
/*! popCommand scan the buffer to a terminating ',' or ';' symbol by removing
    any text between:

    - { and }
    - [ and ]
    - / * and * /
    - // and \\n
    - # and \\n
    - (and )

    This function is interruptible which means that is does not rescan the
    entire buffer from the start each time it is called, but it stores it's
    internal state before quitting and starts again where it left. This
    is important when the buffer comes from a TCP/IP entry connection where
    instructions typically arrive in several shots.

    The final ',' or ';' is the last character of the popped data.

    \param length the length of the extracted command. zero means "no command
	   is available yet".
    \return a pointer to the the data popped or 0 in case of error.
*/
ubyte*
UCommandQueue::popCommand (int &length)
{
  bool   found;
  ubyte* result;
  int    position,nextposition,previousposition;
  char   p0,p1,p_1;

  if (dataSize_ == 0)
  {
    length = 0;
    return buffer_;
  }

  // Scanning

  position = start_ + cursor_;
  if (position >= bufferSize_)
    position = position - bufferSize_;

  nextposition = position + 1;
  if (nextposition >= bufferSize_)
    nextposition = nextposition - bufferSize_;

  previousposition = position - 1;
  if (previousposition < 0)
    previousposition = previousposition + bufferSize_;
  p0 = (char) (*(buffer_ + previousposition)); // extract the previous char.
  if (cursor_ == 0) p0 = ' '; // no previous char at start

  found = false;
  while ((cursor_ < dataSize_ ) &&
	 (!found ) )
  {
    p_1 = p0;
    p0 = (char) (*(buffer_ + position));
    if (cursor_ < dataSize_ - 1)
      p1 = (char) (*(buffer_ + nextposition));
    else p1 = '-';

    if (discard_)
    {
      // One char close sequence
      if ((p0 == closechar_) &&
	  (closechar2_ == ' ') )
      {
	discard_ = false;
	if ((closechar_ == '"') &&
	    (p_1 == '\\'))
	  discard_ = true; // cancel the closure.
      }

      // Two chars close sequence
      if ((p_1 == closechar_ ) &&
	  (p0  == closechar2_ ) &&
	  (closechar2_ != ' '))
	discard_ = false;
    }
    else
    {
      if (p0 == '{') bracketlevel_ ++;
      if (p0 == '}') bracketlevel_ --;
      if (p0 == '[') sbracketlevel_ ++;
      if (p0 == ']') sbracketlevel_ --;
      if (p0 == '(') parenlevel_ ++;
      if (p0 == ')') parenlevel_ --;

      if (bracketlevel_ < 0) bracketlevel_ = 0;
      if (sbracketlevel_ < 0) sbracketlevel_ = 0;
      if (parenlevel_ < 0) parenlevel_ = 0;

      if (p0 == '#')
      {
	discard_    = true;
	closechar_  = '\n';
	closechar2_ = ' ';
      }
      if (p0 == '"')
      {
	discard_    = true;
	closechar_  = '"';
	closechar2_ = ' ';
      }
      if (((p0 == '/') &&
	   (p1 == '*') ) ||
	  ((p_1 == '/') &&
	   (p0 == '*') ))
      {
	discard_    = true;
	closechar_  = '*';
	closechar2_ = '/';
      }
      if (((p0 == '/') &&
	   (p1 == '/') ) ||
	  ((p_1 == '/') &&
	   (p0 == '/') ) )
      {
	discard_    = true;
	closechar_  = '\n';
	closechar2_ = ' ';
      }
    }

    // , or ; separator, except between paren or brackets
    if ((((char)(*(buffer_ + position)) == ',' ) ||
	 ((char)(*(buffer_ + position)) == ';' )) &&
	(!discard_ ) &&
	(bracketlevel_ == 0) &&
	(sbracketlevel_ == 0) &&
	(parenlevel_ == 0) )
      found = true;

    // Emergency escape character: ¤
    if (((char)(*(buffer_ + position)) == '$' ) &&
	((char)(*(buffer_ + nextposition)) == '$' ) &&
	(!discard_ ) )
    {
      length = -1;
      bracketlevel_ = 0;
      sbracketlevel_ = 0;
      parenlevel_ = 0;
      cursor_ = 0;
      discard_  = false;
      return (0);
    }

    cursor_ ++;
    position ++;
    if (position == bufferSize_)
      position = 0;
    nextposition ++;
    if (nextposition == bufferSize_)
      nextposition = 0;

  };

  if (found)
  {
    result = pop(cursor_);
    length = cursor_;
    cursor_ = 0;
  }
  else
  {
    result = buffer_;
    length = 0;
  }

  return result;
}
