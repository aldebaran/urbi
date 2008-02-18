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
  /*! UQueue implements a dynamic circular FIFO buffer.
      You can specify the following parameters:

      \param minBufferSize is the initial size of the buffer. If the buffer is
	   shrinked (adaptive buffer), its size will be at least minBufferSize.
	   The default value if not specified is 4096 octets.
      \param maxBufferSize is the maximal size of the buffer. The buffer size is
	   increased when one tries to push data that is too big for the current
	   buffer size. However, this dynamic behavior is limited to
	   maxBufferSize in any case. If the buffer can still not hold the
	   pushed data, the push() function returns UFAIL.
	   If set to zero, the limit is infinite.
	   The default value is equal to minBufferSize.
      \param adaptive the default behavior of the UQueue is to increase the size
	   of the internal buffer if one tries to hold more data that what the
	   current size allows (up to maxBufferSize, as we said). However, if
	   the buffer is adaptive, it will also be able to shrink the buffer.
	   This is very useful to allow the UQueue to grow momentarily to hold
	   a large amount of data and then recover memory after the boost, if
	   this data size is not the usual functioning load.
	   If adaptive is non null, the behavior is the following: the UQueue
	   counts the number of calls to the pop() function. Each time this
	   number goes above 'adaptive', the UQueue will resize to the maximum
	   data size it has held in the past 'adaptive' calls to pop(), and
	   minBufferSize at least. So, 'adaptive' is a time windows that is
	   periodicaly checked to shink the buffer if necessary.
	   Note that the buffer will shrink only if there is a minimum of 20%
	   size difference, to avoid time consuming reallocs for nothing.
	   We recommand a default value of 100 for the adaptive value.
  */
  explicit UQueue (size_t minBufferSize = 0,
		   size_t maxBufferSize = -1,
		   size_t adaptive = 0);
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
  UErrorValue push (const char *s);

  //! Pushes a buffer into the queue.
  /*! This function tries to store the buffer in the queue, and tries to extend
      to buffer size when the buffer does not fit and if it is possible.

      \param buffer the buffer to push
      \param length the length of the buffer
      \return
	    - USUCCESS: successful
	    - UFAIL   : could not push the buffer. The queue is not changed.
  */
  UErrorValue push (const ubyte *buffer, size_t length);

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
  ubyte* virtualPop (size_t length);

 //! Pops at most 'length' bytes out of the Queue
 /*! Pops at most 'length' bytes. The actual size is returned in 'length'.
     The ubyte* pointer returned is not permanent. You have to make a
     copy of the data pointed if you want to keep it. Any call to a
     member function of UQueue might alter it later.

     This method is called "fast" because is will not use the
     temporary outputBuffer if the requested data is half at the end
     and half at the beginning of the buffer. It will return only the
     part at the end of the buffer and return the actual size of data
     popped. You have to check if this value is equal to the requested
     length. If it is not, a second call to fastPop will give you the
     rest of the buffer, with a different pointer.  This function is
     useful if you plan to pop huge amount of data which will probably
     span over the end of the buffer and which would, with a simple
     call to 'pop', result in a huge memory replication. In other
     cases, prefer 'pop', which is most of the time as efficient as
     fastPop and much more convenient to use.

     \param length the length requested. Contains the actual length popped.
     \return a pointer to the the data popped or 0 in case of error.
 */
  ubyte* fastPop (size_t &length);

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

  //! Available free space in the buffer.
  size_t bufferFreeSpace ();
  //! Max available free space in the buffer.
  size_t bufferMaxFreeSpace();
  //! Size of the data in the buffer
  size_t dataSize ();

  //! Set a mark to be able to revert to this position
  void mark ();

  //! Revert the buffer to the marked position.
  void revert ();
  //! Locked status of the queue
  bool locked ();
  //! Adaptive accessor
  void setAdaptive (size_t adaptive);

private:
  /// Implement buffer adaptive scheme.
  /// \param toPop  size of the current buffer pop.
  // Called by pop() only.
  void adapt(size_t toPop);

  /// Grow \a s, using the threshold maxBufferSize_.
  void enlarge (size_t& s) const;

protected:

  /// Initial size of the output buffer used by pop.
  enum { INITIAL_BUFFER_SIZE = 4096 };

  /// Stores the initial size of the internal buffer.
  size_t minBufferSize_;

  /// Maximum size the dynamical internal buffer is allowed to grow.
  size_t maxBufferSize_;

  /// Size of the time window used by the adaptive algorithm.
  size_t adaptive_;

  /// queue internal buffer (circular).
  std::vector<ubyte> buffer_;

  /// buffer used by pop to return it's value.
  std::vector<ubyte> outputBuffer_;

  /// internal buffer start offset.
  size_t start_;
  /// internal buffer end offset.
  size_t end_;
  /// size of the data inside the buffer.
  size_t dataSize_;

  /// nb calls to the pop() function.
  size_t nbPopCall_;
  /// maximal data size in the adaptive time windows. Used to shrink
  /// the buffer.
  size_t topDataSize_;
  /// maximal data size outputed in the time windows. Used to shrink
  /// outputBuffer.
  size_t topOutputSize_;
  /// mark offset.
  size_t mark_;
  /// lock the connection after a failure (only 'mark' can unlock it)
  bool locked_;
};

# include "uqueue.hxx"

#endif
