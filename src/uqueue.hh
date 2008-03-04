/*! \file uqueue.hh
 *******************************************************************************

 File: uqueue.h\n
 Definition of the UQueue class.

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

#ifndef UQUEUE_HH
# define UQUEUE_HH

# include <deque>
# include <vector>

# include "kernel/utypes.hh"

/// Dynamic expendable and shrinkable circular FIFO buffer
/*! UQueue is a FIFO buffer which is both dynamic and circular.

    This FIFO queue can be filled in (push) or partially emptied (pop).

    * Dynamic means that if the buffer is not big enough to push new data in, it
    will be expanded, up to a given limit (maxBufferSize). After that limit,
    a buffer overflow error will occur and the push will return UFAIL.
    If the adaptive option is used, the buffer will shrink again if its size
    is too big, according to the recent past of data size it has holded. This
    option is fully described in the constructor. It is implemented in the pop()
    function.

    * Circular means that if the data pushed cannot hold at the end of the
    buffer, it will be splitted and looped at the beginning, thus avoiding any
    memory space loss. In other words, the topology of the buffer is such that
    the end connects the beginning.
 */
class UQueue
{
public:

  //! UQueue constructor.
  UQueue ();
  virtual ~UQueue ();

  //! Pushes a string into the queue. The zero ending character is ignored.
  /*! The string is converted into a ubyte* buffer and the function calls the
    push(ubyte*,size_t) function.

    \param s the string to send
    \return
	    - USUCCESS: successful
	    - UFAIL   : could not push the string.
    \sa push(const ubyte*,size_t)
  */
  void push (const char *s);

  //! Pushes a buffer into the queue.
  /*! This function tries to store the buffer in the queue, and tries to extend
      to buffer size when the buffer does not fit and if it is possible.

      \param buffer the buffer to push
      \param length the length of the buffer
  */
  void push (const ubyte *buffer, size_t length);

  //! Pops 'length' bytes out of the Queue
  /*! Pops 'length' bytes.
      The ubyte* pointer returned is not permanent. You have to make a copy
      of the data pointed if you want to keep it. Any call to a member function
      of UQueue might alter it later.

      \param length the length requested.
      \return a pointer to the the data popped or 0 in case of error.
  */
  ubyte* pop (size_t length);

  //! Simulates the pop of 'length' bytes out of the Queue
  /*! Behave like pop but simulate the effect. This is useful if one
      wants to try something with the popped buffer and check the
      validity before actually popping the data.  It is also typically
      used when trying to send bytes in a connection and it is not
      known in advance how many bytes will be effectively sent.

      \param length the length requested.
      \return a pointer to the the data popped or 0 in case of error.
  */
  ubyte* front (size_t length);

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
  std::string pop_command ();

  //! Clear the queue
  void clear ();

  /// Whether it's empty.
  bool empty () const;

  //! Size of the data in the buffer
  size_t size () const;

private:
  /// queue internal buffer (circular).
  std::deque<ubyte> buffer_;

  /// buffer used by pop to return it's value.
  std::vector<ubyte> outputBuffer_;
};

# include "uqueue.hxx"

#endif
