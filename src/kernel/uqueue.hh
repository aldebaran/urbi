/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file kernel/uqueue.hh

#ifndef KERNEL_UQUEUE_HH
# define KERNEL_UQUEUE_HH

# include <libport/fifo.hh>
# include <string>

namespace kernel
{
  class UQueue : public libport::Fifo<char, '\0'>
  {
  public:
    typedef libport::Fifo<char, '\0'> super_type;

    //! UQueue constructor.
    //
    // \param chunk_size The size of allocated chunks. The buffer will start
    //                   at that size and increment by the same amount when
    //                   needed.
    UQueue(super_type::size_type chunk_size = 1024);

    using super_type::push;
    void push(const std::string& s);

    //! Pops the next command in the queue.
    /*! Scan the buffer to a terminating ',' or ';' symbol by removing
     any text between:

     - { and }
     - [ and ]
     - / * and * /
     - // and \\n
     - # and \\n
     - ( and )

     The final ',' or ';' is the last character of the popped data.

     \return the command popped or an empty string if there was an error or
	   nothing to pop.
     */
    std::string pop_command();

  private:
    /// Do not try to pop a command if less than preparse_hint bytes are
    /// available.
    size_t preparse_hint;
  };
}

# include <kernel/uqueue.hxx>

#endif // !KERNEL_UQUEUE_HH
