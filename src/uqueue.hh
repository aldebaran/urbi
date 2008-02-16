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

  explicit UQueue  (size_t minBufferSize = 0,
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
  UErrorValue         push              (const char *s);
  UErrorValue         push              (const ubyte *buffer, size_t length);

  ubyte*              pop               (size_t length);
  ubyte*              virtualPop        (size_t length);
  ubyte*              fastPop           (size_t &length);

  void                clear             ();

  //! Available free space in the buffer.
  size_t                 bufferFreeSpace   ();
  //! Max available free space in the buffer.
  size_t                 bufferMaxFreeSpace();
  //! Size of the data in the buffer
  size_t                 dataSize          ();

  void                mark              ();
  void                revert            ();
  //! Locked status of the queue
  bool                locked            ();
  //! Adaptive accessor
  void                setAdaptive       (size_t adaptive);

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
