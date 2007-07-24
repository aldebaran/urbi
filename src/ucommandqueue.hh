/*! \file ucommandqueue.hh
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

#ifndef UCOMMANDQUEUE_HH
# define UCOMMANDQUEUE_HH

# include <vector>

# include "kernel/fwd.hh"

# include "uqueue.hh"

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
  explicit UCommandQueue  (int minBufferSize = 0,
                           int maxBufferSize = -1,
                           int adaptive = 0);

  virtual ~UCommandQueue ();

  //! Pops the next command in the queue.
  /*! Scan the buffer to a terminating ',' or ';' symbol by removing
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

   \param length   of the extracted command. zero means "no command
		   is available yet".
   \return a pointer to the the data popped or 0 in case of error.
   */
  ubyte*              popCommand        (int &length);

private:

  /// internal position of the preparsing cursor. It's an offset of
  /// start_
  int            cursor_;
  /// True when a commentary is active;
  bool           discard_;
  /// Used to store the 1st closing character for the commentary
  /// detection.
  char           closechar_;
  /// Used to store thezq 2nd closing character for the commentary
  /// detection.
  char           closechar2_;
  /// A stack of expected closing braces: ), ], } etc.
  std::vector<char> closers_;
};

#endif // !UCOMMANDQUEUE_HH
