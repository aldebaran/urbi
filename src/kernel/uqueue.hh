/*! \file kernel/uqueue.hh
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

#ifndef KERNEL_UQUEUE_HH
# define KERNEL_UQUEUE_HH

# include <string>

/*! UQueue is an efficient FIFO queue.

    This FIFO queue can be filled in (push) or partially emptied (pop).
    Data will be moved around only when this is necessary. It works like
    this:

        +-------------------------------------------------+
        | . . . . . . . . . X X X X X X X 0 . . . . . . . |
        +-^-----------------^-------------^---------------+
          |                 |             |
          |                 |             +--- next position to write
          |                 |
          |                 +--- first data character
          |
          +--- beginning of the storage area

    New data is entered after the existing one (marked by X), and a
    NULL character is added at the end so that the whole buffer can be
    considered as being a C string when retrieved as a block. Data is
    always kept in consecutive positions and in the right order, making
    it easy to see it as an array in memory.

    If there is not enough room after the existing data, the data will
    be moved to the beginning of the buffer, after a reallocation if
    necessary.

    When data is retrieved, the position of the first character to read
    is moved forward. If the buffer happens to be emptied, the markers
    will be reset to the beginning of the buffer so that we are less
    likely to require a data relocation in the future.
 */
class UQueue
{
public:

  //! UQueue constructor.
  //
  // \param chunk_size The size of allocated chunks. The buffer will start
  //                   at that size and increment by the same amount when
  //                   needed.
  UQueue(size_t chunk_size = 1024);

  //! UQueue destructor.
  virtual ~UQueue();

  //! Pushes a string into the queue. The zero ending character is ignored.
  /*! The string is converted into a char* buffer and the function calls the
    push(char*,size_t) function.

    \param s the string to send

    \sa push(const char*,size_t)
  */
  void push(const char *s);

  //! Pushes a buffer into the queue.
  /*! This function tries to store the buffer in the queue, and tries to extend
      to buffer size when the buffer does not fit and if it is possible.

      \param buffer the buffer to push
      \param length the length of the buffer
  */
  void push(const char *buffer, size_t length);

  //! Pops 'length' bytes out of the Queue
  /*! Pops 'length' bytes.
      The char* pointer returned is not permanent. You have to make a
      copy of the data pointed if you want to keep it around calls to
      the push() methods. The data is not null-terminated unless the whole
      content is requested.

      \param length the length requested.
      \return a pointer to the the data popped or 0 in case of error.
  */
  const char* pop (size_t length);

  //! Simulates the pop of 'length' bytes out of the Queue
  /*! Behave like pop but do not consume the data. This is useful if
      one wants to try something with the popped buffer and check the
      validity before actually popping the data.  It is also typically
      used when trying to send bytes in a connection and it is not
      known in advance how many bytes will be effectively sent.  The
      data is not null-terminated unless the whole content is
      requested.

      \param length the length requested.
      \return a pointer to the the data popped or 0 in case of error.
  */
  const char* front (size_t length) const;

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
  std::string pop_command();

  //! Clear the queue
  void clear();

  /// Whether it's empty.
  bool empty() const;

  //! Size of the data in the buffer
  size_t size() const;

private:
  /// Chunk size.
  const size_t chunk_size_;

  /// Internal buffer.
  char* buffer_;

  /// Reserved size.
  size_t capacity_;

  /// Address of first character in the buffer.
  char* first_character_;

  /// Address of next character to write, following the last one
  /// in the buffer.
  char* next_character_;
};

# include <kernel/uqueue.hxx>

#endif // !KERNEL_UQUEUE_HH
