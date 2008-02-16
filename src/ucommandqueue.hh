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
# include <string>

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
  explicit UCommandQueue  (size_t minBufferSize = 0,
			   size_t maxBufferSize = -1,
			   size_t adaptive = 0);

  virtual ~UCommandQueue ();

  //! Pops the next command in the queue.
  /*! Scan the buffer to a terminating ',' or ';' symbol by removing
   any text between:

   - { and }
   - [ and ]
   - / * and * /
   - // and \\n
   - # and \\n
   - ( and )

   This function is interruptible which means that is does not rescan the
   entire buffer from the start each time it is called, but it stores it's
   internal state before quitting and starts again where it left. This
   is important when the buffer comes from a TCP/IP entry connection where
   instructions typically arrive in several shots.

   The final ',' or ';' is the last character of the popped data.

   \return the command popped or an empty string if there was an error or
	   nothing to pop.
   */
  std::string popCommand ();

private:

  /// start_.  Should always be before a complete "character".  I.e.,
  /// should never point to the * of an "/*" that does open a comment,
  /// should stop between a `\' and its "argument" and so forth.
  size_t cursor_;

  /// The closing character(s) for the commentary/string detection.
  /// 0 if not in such a token.
  /// Long lexemes: /* ... */, " ... ", # ... \n, \\ ... \n.
  const char* close_;

  /// A stack of expected closing braces: ), ], } etc.
  std::vector<char> closers_;
};

#endif // !UCOMMANDQUEUE_HH
