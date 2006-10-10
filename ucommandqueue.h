/*! \file ucommandqueue.h
 *******************************************************************************

 File: ucommandqueue.h\n
 Definition of the UCommandQueue class.

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

#ifndef UCOMMANDQUEUE_H_DEFINED
#define UCOMMANDQUEUE_H_DEFINED

#include "fwd.hh"
#include "uqueue.h"

/// UQueue with facilities to pop a well formed command.
/*! UCommandqueue is a UQueue with the popCommand(int &length) function to
    pre-parse the internal buffer looking for a well formed command.
    If it finds one, it pops it out.
    popCommand is interruptible which means that it can process a partial
    buffer, stop, and carry on later where it stopped.
 */
class UCommandQueue : public UQueue
{
public:

  UCommandQueue  (int minBufferSize = 0,
		  int maxBufferSize = -1,
		  int adaptive = 0);

  virtual ~UCommandQueue ();

  ubyte*              popCommand        (int &length);

protected:

  int            cursor_;        ///< internal position of the preparsing cursor
				 ///< it's an offset of start_
  int            bracketlevel_;  ///< Stores how many brackets are open
  int            sbracketlevel_; ///< Stores how many square brackets are open
  int            parenlevel_;    ///< Stores how many parenthesis are open
  bool           discard_;       ///< True when a commentary is active;
  char           closechar_;     ///< used to store the 1st closing character
				 ///< for the commentary detection.
  char           closechar2_;    ///< used to store thezq 2nd closing character
				 ///< for the commentary detection.
};

#endif
