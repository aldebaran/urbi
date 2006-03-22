/*! \file umixqueue.h
 *******************************************************************************

 File: umixqueue.h\n
 Definition of the UMixQueue class.

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

#ifndef UMIXQUEUE_H_DEFINED
#define UMIXQUEUE_H_DEFINED

#include "uqueue.h"

class UServer;

/// UQueue with facilities to push data in mixed mode (adding) 
/*! UMixqueue is a UQueue with the pushMix(const ubyte *buffer, int length) function 
    that pushes the data at the beginning of the buffer (instead of "at the end")
    and do a summation over the existing data in the buffer.
    Typical use of this object is for soundqueues in "mix" mode.
 */
class UMixQueue : public UQueue
{
public:
	
  UMixQueue  (int minBufferSize = 0,
              int maxBufferSize = -1,
              int adaptive = 0);

  virtual ~UMixQueue ();

  UErrorValue         pushMix        (const ubyte *buffer, 
                                      int length,
                                      UBlend blend);

protected:

  int            cursor_;        ///< internal position of the preparsing cursor
                                 ///< it's an offset of start_
  int            bracketlevel_;  ///< Stores how many brackets are open
  int            parenlevel_;    ///< Stores how many parenthesis are open
  bool           discard_;       ///< True when a commentary is active;
};

#endif
