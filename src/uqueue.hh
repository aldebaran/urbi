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

  explicit UQueue  (int minBufferSize = 0,
		    int maxBufferSize = -1,
		    int adaptive = 0);

  virtual ~UQueue ();

  //! Pushes a string into the queue. The zero ending character is ignored.
  /*! The string is converted into a ubyte* buffer and the function calls the
    push(ubyte*,int) function.

    \param s the string to send
    \return
	    - USUCCESS: successful
	    - UFAIL   : could not push the string.
    \sa push(const ubyte*,int)
  */
  UErrorValue         push              (const char *s);
  UErrorValue         push              (const ubyte *buffer, int length);

  ubyte*              pop               (int length);
  ubyte*              virtualPop        (int length);
  ubyte*              fastPop           (int &length);

  void                clear             ();

  //! Available free space in the buffer.
  int                 bufferFreeSpace   ();
  //! Max available free space in the buffer.
  int                 bufferMaxFreeSpace();
  //! Size of the data in the buffer
  int                 dataSize          ();

  void                mark              ();
  void                revert            ();
  //! Locked status of the queue
  bool                locked            ();
  //! Adaptive accessor
  void                setAdaptive       (int adaptive);

protected:

  /// Initial size of the output buffer used by pop.
  enum { INITIAL_BUFFER_SIZE = 4096 };

  /// Stores the initial size of the internal buffer.
  int minBufferSize_;

  /// Maximum size the dynamical internal buffer is allowed to grow.
  int maxBufferSize_;

  /// Size of the time window used by the adaptive algorithm.
  int adaptive_;
  /// current internal buffer size.
  int bufferSize_;
  /// queue internal buffer (circular).
  std::vector<ubyte> buffer_;

  /// size of the output buffer used by pop.
  int outputBufferSize_;
  /// buffer used by pop to return it's value.
  std::vector<ubyte> outputBuffer_;

  /// internal buffer start offset.
  int start_;
  /// internal buffer end offset.
  int end_;
  /// size of the data inside the buffer.
  int dataSize_;

  /// nb calls to the pop() function.
  int nbPopCall_;
  /// maximal data size in the adaptive time windows. Used to shrink
  /// the buffer.
  int topDataSize_;
  /// maximal data size outputed in the time windows. Used to shrink
  /// outputBuffer.
  int topOutputSize_;
  /// mark offset.
  int mark_;
  /// lock the connection after a failure (only 'mark' can unlock it)
  bool locked_;
};

# include "uqueue.hxx"

#endif
